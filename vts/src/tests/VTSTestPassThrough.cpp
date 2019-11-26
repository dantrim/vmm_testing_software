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
#include <math.h> // pow
using namespace std;

//logging
#include "spdlog/spdlog.h"

//ROOT
#include "TH2F.h"

namespace vts
{


bool VTSTestPassThrough::initialize(const json& config)
{

    stringstream msg;
    msg << "Initializing with config: " << config.dump();
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

    m_test_steps.clear();
    vector<string> pulse_widths = { "1", "2", "4", "8", "10", "12" };
    for(size_t i = 0; i < pulse_widths.size(); i++)
    {
        TestStep t;
        t.pulse_width = pulse_widths.at(i);
        m_test_steps.push_back(t);
    }

    // initialize histo
    h2_channel_hit_vs_pdo = new TH2F("h2_chan_vs_pdo", "Pulse Width 1;Channel Number;PDO [counts]", 64, 0, 64, 100, 0, -1);
    h2_channel_hit_vs_tdo = new TH2F("h2_chan_vs_tdo", "Pulse Width 1;Channel Number;TDO [counts]", 64, 0, 64, 100, 0, -1);
    
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestPassThrough::load()
{
    set_current_state( get_current_state() + 1);

    return true;
}

bool VTSTestPassThrough::configure()
{
    //reset_vmm();
    TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    //fpga_triggers["latency"] = "40";
    fpga_clocks["cktp_width"] = t.pulse_width;

    comm()->configure_fpga(fpga_triggers, fpga_clocks);

    // build the VMM config info for this step
    json vmm_config = m_base_vmm_config;
    json vmm_spi = vmm_config.at("vmm_spi");
    json vmm_globals = vmm_spi.at("global_registers");
    json vmm_channels = vmm_spi.at("channel_registers");

    vector<vts::vmm::Channel> channels; 
    for(size_t i = 0; i < 64; i++)
    {
        bool st = true;
        bool sm =  false;
        bool smx = false;
        json jch = {{"id",i},{"sc",false},{"sl",false},{"sth",false},{"st",st},{"sm",sm},{"smx",smx},{"sd",0}};
        vmm::Channel ch; ch.load(jch);
        channels.push_back(ch);
    }
    auto chvec = vts::vmm::channel_vec_to_json_config(channels); 
    auto channel_registers = chvec.at("channel_registers");
    vmm_spi["channel_registers"] = channel_registers;

    vmm_globals["sm5"] = "0";
    vmm_globals["sbmx"] = "DISABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "ENABLED";
    //vmm_globals["sdt_dac"] = "100";
    //vmm_globals["sdp_dac"] = "300";
    //vmm_globals["sg"] = "16.0";
    vmm_spi["global_registers"] = vmm_globals;

    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration
    comm()->configure_vmm(vmm_config, false);

    return true;
}

bool VTSTestPassThrough::run()
{
    // reset the event counters for this new run
    reset_event_count();
    comm()->acq_toggle(1);
    // keep running until data processing has completed
    while(processing_events())
    {
        if(!processing_events()) break;
        continue;
    }
    comm()->acq_toggle(0);
    return true;
}

bool VTSTestPassThrough::process_event(vts::daq::DataFragment* fragment)
{
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        return false;
    }
    TestStep t = m_test_steps.at(get_current_state() - 1);
    string pulse_width = t.pulse_width;
    //float pulse_width_val = std::stof(pulse_width);

    vector<vts::decode::vmm::Sample> samples = vts::decode::vmm::decode(fragment->packet);
    for(const auto & sample : samples)
    {
        if(n_events_processed() >= n_events_per_step()) break;
        if(t.pulse_width == "1")
        {
            auto pdo = sample.pdo();
            auto tdo = sample.tdo();
            auto channel = sample.channel();
            h2_channel_hit_vs_pdo->Fill(channel, pdo);
            h2_channel_hit_vs_tdo->Fill(channel, tdo);
        }
    }
    check_status();
    event_processed();
    return true;
}


bool VTSTestPassThrough::analyze()
{
    return true;
}

bool VTSTestPassThrough::analyze_test()
{
    return true;
}

bool VTSTestPassThrough::finalize()
{
    vector<TH2F*> histos = { h2_channel_hit_vs_pdo, h2_channel_hit_vs_tdo };
    bool all_ok = true;
    for(auto & h : histos)
    {
        if(!h)
        {
            h = 0;
            all_ok = false;
        }
        if(!store(h))
        {
            log->warn("{0} - Failed to store histogram with nae \"{1}\"",__VTFUNC__,h->GetName());
            all_ok = false;
        }
        if(all_ok)
        {
            delete h;
            h = 0;
        }
    }
    if(!all_ok)
    {
        return false;
    }
    else
    {
        return true;
    }
}

json VTSTestPassThrough::get_results()
{
    json jresults = {
        {"RESULT",VTSTestResultToStr(VTSTestResult::SUCCESS)}
    };
    return jresults;
}

void VTSTestPassThrough::check_status()
{
    float frac = event_fraction_processed();
    emit signal_status_update(frac);
}

void VTSTestPassThrough::reset_vmm()
{
    comm()->configure_vmm(m_base_vmm_config, /*perform reset*/ true);
}

} // namespace vts
