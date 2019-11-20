//vts
#include "communicator_frontend.h" // for sending commands to the frontend
#include "tests/VTSTestBaselines.h"
#include "helpers.h"
#include "frontend_struct.h"
#include "daq_defs.h"
//decoding
#include "vts_decode.h"

//std/stl
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
using namespace std;

//logging
#include "spdlog/spdlog.h"

namespace vts
{


bool VTSTestBaselines::initialize(const json& config)
{

    stringstream msg;
    msg << "Initializing with config: " << config.dump();
    log->info("{0} - {1}",__VTFUNC__,msg.str());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    msg.str("");
    msg << "INITIALIZED!";
    log->info("{0} - {1}",__VTFUNC__,msg.str());

    m_test_data = m_test_config.at("test_data");
    // get the base configs for the fpga and VMM
    string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    string vmm_file = m_test_data.at("base_config_global").get<std::string>();

    // load FPGA base config
    std::ifstream ifs_fpga(fpga_file);
    json fpga_data;
    ifs_fpga >> fpga_data;
    m_base_fpga_config = fpga_data;

    // load VMM base config 
    std::ifstream ifs_vmm(vmm_file); 
    json vmm_data;
    ifs_vmm >> vmm_data;
    m_base_vmm_config = vmm_data;

    // dummy
    m_test_steps.clear();
    stringstream ch;
    for(int i = 0; i < 64; i++)
    {
        ch.str("");
        ch << i;
        TestStep t;
        t.channel = ch.str();
        m_test_steps.push_back(t);
    }
    
    set_current_state(0);
    set_n_states(m_test_steps.size());
    return true;
}

bool VTSTestBaselines::load()
{
    set_current_state( get_current_state() + 1);

    log->info("{0}",__VTFUNC__);
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool VTSTestBaselines::configure()
{
    log->info("{0}",__VTFUNC__);

    //reset_vmm();
    TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    fpga_triggers["latency"] = "0";

    comm()->configure_fpga(fpga_triggers, fpga_clocks);
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // build the VMM config info for this step
    json vmm_config = m_base_vmm_config;
    json vmm_spi = vmm_config.at("vmm_spi");
    json vmm_globals = vmm_spi.at("global_registers");
    json vmm_channels = vmm_spi.at("channel_registers");

    vector<vts::vmm::Channel> channels; 
    for(size_t i = 0; i < 64; i++)
    {
        bool st = false;
        bool sm =  false; //(i==14) ? false : true;
        bool smx = false; //(i==14) ? true : false;
        json jch = {{"id",i},{"sc",false},{"sl",false},{"sth",false},{"st",st},{"sm",sm},{"smx",smx},{"sd",0}};
        vmm::Channel ch; ch.load(jch);
        channels.push_back(ch);
    }
    auto chvec = vts::vmm::channel_vec_to_json_config(channels); 
    auto channel_registers = chvec.at("channel_registers");
    vmm_spi["channel_registers"] = channel_registers;

    vmm_globals["sm5"] = t.channel;
    vmm_globals["sbmx"] = "ENABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "DISABLED";
    vmm_globals["sdt_dac"] = "100";
    vmm_globals["sdp_dac"] = "400";
    vmm_globals["sg"] = "1.0";
    vmm_spi["global_registers"] = vmm_globals;

    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration
    comm()->configure_vmm(vmm_config, false);

    return true;
}

bool VTSTestBaselines::run()
{
    // reset the event counters for this new run
    reset_event_count();

    bool status = comm()->sample_xadc(n_events_per_step(), 500);

    // keep running until data processing has completed
    while(processing_events())
    {
        if(!processing_events()) break;
        continue;
    }
    return true;
}

bool VTSTestBaselines::process_event(vts::daq::DataFragment* fragment)
{
    if(n_events_processed() >= n_events_per_step())
    {
        log->info("{0} - Event count reached ({1})",__VTFUNC__, n_events_processed());
        return false;
    }

    vector<vts::decode::xadc::Sample> samples = vts::decode::xadc::decode(fragment->packet);

    if(true)
    {
        TestStep t = m_test_steps.at(get_current_state() - 1);
        string current_channel = t.channel;

        int n_samples = samples.size();
        uint32_t avg_sample = 0;
        for(const auto & sample : samples) avg_sample += sample.sample();
        avg_sample = float(avg_sample);
        avg_sample /= float(n_samples);
        
        stringstream msg;
        msg << "Average xadc sampling for " << n_samples << " for CHANNEL " << current_channel << " = " << std::dec << avg_sample;
        log->info("{0} - {1}",__VTFUNC__,msg.str());
    }
    for(const auto& s: samples) event_processed();
    comm()->sample_xadc(n_events_per_step(), 500);
    return true;
}

bool VTSTestBaselines::analyze()
{
    log->info("{0}",__VTFUNC__);
    return true;
}

bool VTSTestBaselines::analyze_test()
{
    log->info("{0}",__VTFUNC__);
    return true;
}

bool VTSTestBaselines::finalize()
{
    log->info("{0}",__VTFUNC__);
    return true;
}

json VTSTestBaselines::get_results()
{
    json jresults = {
        {"RESULT",VTSTestResultToStr(VTSTestResult::SUCCESS)}
    };
    return jresults;
}

void VTSTestBaselines::reset_vmm()
{
    //vts::CommunicatorFrontEnd comm;
    //comm.load_config(m_frontend_config);
    //comm.configure_vmm(m_base_vmm_config, /*perform reset*/ true);
    comm()->configure_vmm(m_base_vmm_config, /*perform reset*/ true);
}

} // namespace vts
