//vts
#include "communicator_frontend.h" // for sending commands to the frontend
#include "tests/VTSTestPassThrough.h"
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


bool VTSTestPassThrough::initialize(const json& config)
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
    //vector<string> threshold_dacs = { "100", "200", "400", "600", "800", "1000" };
    //vector<string> pulser_dacs = { "300", "400", "500", "600", "700", "800", "900", "1000" };
    vector<string> pulse_widths = { "1", "2", "4", "8", "10", "12" };
    m_test_steps.clear();
    for(size_t i = 0; i < pulse_widths.size(); i++)
    {
        TestStep t;
        t.pulse_width = pulse_widths.at(i);
        //t.global_pulser_dac = pulser_dacs.at(i);
        m_test_steps.push_back(t);
    } // i
    
    set_current_state(0);
    set_n_states(m_test_steps.size());
    return true;
}

bool VTSTestPassThrough::load()
{
    set_current_state( get_current_state() + 1);

    log->info("{0}",__VTFUNC__);
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool VTSTestPassThrough::configure()
{
    log->info("{0}",__VTFUNC__);

    reset_vmm();
    // establish the communicator
//    vts::CommunicatorFrontEnd comm;
//    comm.load_config(m_frontend_config);

    // get the configuration step for this one
//    TestStep t = m_test_steps.at(get_current_state() - 1);
//
//    // configure the fpga
//    json fpga_config = m_base_fpga_config;
//    json fpga_registers = fpga_config.at("fpga_registers");
//    json fpga_triggers = fpga_registers.at("trigger");
//    json fpga_clocks = fpga_registers.at("clocks");
//    fpga_clocks["cktp_width"] = t.pulse_width;
//
//    comm.configure_fpga(fpga_triggers, fpga_clocks);
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//    // build the VMM config info for this step
//    json vmm_config = m_base_vmm_config;
//    json vmm_spi = vmm_config.at("vmm_spi");
//    json vmm_globals = vmm_spi.at("global_registers");
//    json vmm_channels = vmm_spi.at("channel_registers");
//
//    // monitor and pulse channel 14
//
//    vector<vts::vmm::Channel> channels; 
//    for(size_t i = 0; i < 64; i++)
//    {
//        bool st = ( (i==14 || i==28) ? true : false);
//        bool sm = ( (i==14 || i==28) ? false : true);
//        json jch = {{"id",i},{"sc",false},{"sl",false},{"sth",false},{"st",st},{"sm",sm},{"smx",false},{"sd",0}};
//        vmm::Channel ch; ch.load(jch);
//        channels.push_back(ch);
//    }
//    auto chvec = vts::vmm::channel_vec_to_json_config(channels); 
//    auto channel_registers = chvec.at("channel_registers");
//    vmm_spi["channel_registers"] = channel_registers;
//
//    // setup monitoring
//    vmm_globals["sm5"] = "14";
//    vmm_globals["scmx"] = "ENABLED";
//    vmm_globals["sbfm"] = "ENABLED";
//    vmm_globals["sdp_dac"] = "400"; //t.global_pulser_dac;
//    vmm_globals["sdt_dac"] = "200";
//    vmm_spi["global_registers"] = vmm_globals;
//
//    vmm_config["vmm_spi"] = vmm_spi;
//
//    // first reset
//    bool ok = comm.configure_vmm(vmm_config, /*perform reset*/ true);
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//    // now send configuration
//    ok = comm.configure_vmm(vmm_config, false);
//
//
//    //json vmm_config = m_base_vmm_config;
//    //json vmm_spi = vmm_config.at("vmm_spi");
//
//    //json vmm_globals = vmm_spi.at("global_registers");
//    //json vmm_channels = vmm_spi.at("channel_registers");
//    //vmm_globals["sdt_dac"] = t.global_threshold_dac;
//    //vmm_globals["sm5"] = "2"; // we want to observe the global threshold changing
//    //vmm_globals["scmx"] = "DISABLED"; // make it global not channel
//
//    //vmm_spi["global_registers"] = vmm_globals;
//    //vmm_channels["st"] = "0x0";
//    //vmm_spi["channel_registers"] = vmm_channels;
//
//    //vmm_config["vmm_spi"] = vmm_spi;
//
//    //// first reset the VMM
//    //bool ok = comm->configure_vmm(vmm_config, /*perform reset*/ true);
//    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
//
//    //// now set the threshold
//    //ok = comm->configure_vmm(vmm_config, /*perform reset*/ false);
//
//    //std::this_thread::sleep_for(std::chrono::seconds(2));

    ///////////////////////////////////////////////////////////////
    // xadc test
    ///////////////////////////////////////////////////////////////
    TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    fpga_clocks["cktp_width"] = t.pulse_width;
    fpga_triggers["latency"] = "0";

    comm()->configure_fpga(fpga_triggers, fpga_clocks);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // build the VMM config info for this step
    json vmm_config = m_base_vmm_config;
    json vmm_spi = vmm_config.at("vmm_spi");
    json vmm_globals = vmm_spi.at("global_registers");
    json vmm_channels = vmm_spi.at("channel_registers");

    // monitor and pulse channel 14

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

    // setup monitoring
    vmm_globals["sm5"] = "14";
    vmm_globals["sbmx"] = "ENABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "DISABLED";
    vmm_globals["sdp_dac"] = "400"; //t.global_pulser_dac;
    vmm_globals["sdt_dac"] = "200";
    vmm_spi["global_registers"] = vmm_globals;

    vmm_config["vmm_spi"] = vmm_spi;

    // first reset
    bool ok = comm()->configure_vmm(vmm_config, /*perform reset*/ true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // now send configuration
    ok = comm()->configure_vmm(vmm_config, false);

    return true;
}

bool VTSTestPassThrough::run()
{
    // reset the event counters for this new run
    reset_event_count();

    // establish the communicator
    //vts::CommunicatorFrontEnd comm;
    //comm.load_config(m_frontend_config);
    //bool status = comm.acq_toggle(1);
    //bool status = comm.sample_xadc(n_events_per_step());
    bool status = comm()->sample_xadc(n_events_per_step());

    // keep running until data processing has completed
    while(processing_events())
    {
        if(!processing_events()) break;
        continue;
    }
    log->info("{0} - Turning ACQ OFF",__VTFUNC__);
    //status = comm.acq_toggle(0);
    return true;
}

bool VTSTestPassThrough::process_event(vts::daq::DataFragment* fragment)
{
    if(n_events_processed() >= n_events_per_step())
    {
        log->info("{0} - Event count reached ({1})",__VTFUNC__, n_events_processed());
        return false;
    }

    //vector<vts::decode::vmm::Sample> samples = vts::decode::vmm::decode(fragment->packet);
    vector<vts::decode::xadc::Sample> samples = vts::decode::xadc::decode(fragment->packet);

    //if(n_events_processed()%10==0)
    if(true)
    {
        stringstream msg;
        msg << "Fragment size: " << fragment->packet.size() << " event count: " << n_events_processed();
        log->info("{0} - {1}",__VTFUNC__,msg.str());

        stringstream sx;
        sx << " N XADC SAMPLES = " << std::dec << samples.size();
        //sx << " (" << std::dec << sample.vmm_id() << ":" << std::dec << sample.sample() << ")";
        log->info("{0} - {1}: {2}",__VTFUNC__,"Received xadc data", sx.str());

        //stringstream sx;
        //for(const auto & sample : samples)
        //{
        //    sx << " " << sample.channel() << "(trig: " << std::hex << (unsigned)sample.header().trigger_counter() << ")";// << " ART channel: " << std::dec << sample.header().art_address();
        //}
        //log->info("{0} - {1}: {2}",__VTFUNC__,"Received data from channels",sx.str());
    }
    for(const auto& s: samples) event_processed();
    //event_processed();
    comm()->sample_xadc(n_events_per_step());
//    vts::CommunicatorFrontEnd comm;
//    comm.load_config(m_frontend_config);
//    //bool status = comm.acq_toggle(1);
//    bool status = comm.sample_xadc(n_events_per_step());
    //return false;
    return true;
}

bool VTSTestPassThrough::analyze()
{
    log->info("{0}",__VTFUNC__);
    return true;
}

bool VTSTestPassThrough::analyze_test()
{
    log->info("{0}",__VTFUNC__);
    return true;
}

bool VTSTestPassThrough::finalize()
{
    log->info("{0}",__VTFUNC__);
    return true;
}

json VTSTestPassThrough::get_results()
{
    json jresults = {
        {"RESULT",VTSTestResultToStr(VTSTestResult::SUCCESS)}
    };
    return jresults;
}

void VTSTestPassThrough::reset_vmm()
{
    //vts::CommunicatorFrontEnd comm;
    //comm.load_config(m_frontend_config);
    //comm.configure_vmm(m_base_vmm_config, /*perform reset*/ true);
    comm()->configure_vmm(m_base_vmm_config, /*perform reset*/ true);
}

} // namespace vts
