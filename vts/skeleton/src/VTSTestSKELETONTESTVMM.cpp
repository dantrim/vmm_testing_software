//vts
#include "communicator_frontend.h"
#include "tests/VTSTestSKELETONTEST.h"
#include "helpers.h"
#include "frontend_struct.h"
#include "daq_defs.h"
#include "vts_decode.h"

//std/stl
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

//logging
#include "spdlog/spdlog.h"

namespace vts
{


bool VTSTestSKELETONTEST::initialize(const json& config)
{
    stringstream msg;
    msg << "Initializing with config: " << config.dump();
    log->debug("{0} - {1}",__VTFUNC__,msg.str());

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

    // initialize the test recipe here
    m_test_steps.clear();
    vector<string> pulse_widths = { "1", "2", "4", "8", "10", "12" };
    vector<string> dac_pulsers = { "250", "300", "400", "500" };
    for(size_t i = 0; i < pulse_widths.size(); i++)
    {
        for(size_t j = 0; j < dac_pulsers.size(); j++)
        {
            TestStep t;
            t.pulse_width = pulse_widths.at(i);
            t.dac_pulser = dac_pulsers.at(j);
            m_test_steps.push_back(t);
        } // j
    } // i

    // initialize all counters
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestSKELETONTEST::load()
{
    set_current_state( get_current_state() + 1);
    return true;
}

bool VTSTestSKELETONTEST::configure()
{
    TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    fpga_clocks["cktp_width"] = t.pulse_width;
    comm()->configure_fpga(fpga_triggers, fpga_clocks);

    // build the VMM config info for this step
    json vmm_config = m_base_vmm_config;
    json vmm_spi = vmm_config.at("vmm_spi");
    json vmm_globals = vmm_spi.at("global_registers");
    json vmm_channels = vmm_spi.at("channel_registers");

    // pulse all channels
    vector<vts::vmm::Channel> channels; 
    for(size_t i = 0; i < 64; i++)
    {
        json jch = {{"id",i},
                    {"sc",false},
                    {"sl",false},
                    {"sth",false},
                    {"st",true},
                    {"sm",false},
                    {"smx",false},
                    {"sd",0}};
        vmm::Channel ch;
        ch.load(jch);
        channels.push_back(ch);
    }
    vmm_spi["channel_registers"] = vts::vmm::channel_vec_to_json_config(channels).at("channel_registers");

    // set global VMM configuration
    vmm_globals["sm5"] = "4"; // monitor channel 0
    vmm_globals["sbmx"] = "ENABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "DISABLED";
    vmm_globals["sdt_dac"] = "200"; // fix the threshold
    vmm_globals["sdp_dac"] = t.dac_pulser;
    vmm_globals["sg"] = "1.0";
    vmm_spi["global_registers"] = vmm_globals;
    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration SPI string
    comm()->configure_vmm(vmm_config, /*perform reset*/ false);
    return true;
}

bool VTSTestSKELETONTEST::run()
{
    // reset the event counters for this new test step
    reset_event_count();

    // start acq
    comm()->acq_toggle(true);

    // keep running until data processing has completed
    while(processing_events())
    {
        if(!processing_events()) break;
        continue;
    }

    // stop acq now that we're done
    comm()->acq_toggle(false);
    return true;
}

bool VTSTestSKELETONTEST::process_event(vts::daq::DataFragment* fragment)
{
    // return false to stop DAQ and move to next step in the testing
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        return false;
    }

    // get the configuration for thist test step
    TestStep t = m_test_steps.at(get_current_state() - 1);
    string current_pulse_width = t.pulse_width;
    string current_pulse_amplitude = t.dac_pulser;

    // decode the data packet
    vector<vts::decode::vmm::Sample> samples = vts::decode::vmm::decode(fragment->packet);
    for(const auto & sample : samples)
    {
        if(n_events_processed() >= n_events_per_step()) break;
        // process the samples (fill histograms/etc)
    }

    // increment the event counter to move forward in the test (REQUIRED)
    event_processed();
    // signal the event processing status to anyone listing (e.g. progress bar)
    check_status();
    return true;
}


bool VTSTestSKELETONTEST::analyze()
{
    TestStep t = m_test_steps.at(get_current_state() - 1);
    return true;
}

bool VTSTestSKELETONTEST::analyze_test()
{
    return true;
}

bool VTSTestSKELETONTEST::finalize()
{
    return true;
}

json VTSTestSKELETONTEST::get_results()
{
    json jresults = {
        {"RESULT",VTSTestResultToStr(VTSTestResult::SUCCESS)}
    };
    return jresults;
}

} // namespace vts
