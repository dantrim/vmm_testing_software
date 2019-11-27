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
    stringstream ch_id;
    for(size_t i = 0; i < 64; i++)
    {
        ch_id.str("");
        ch_id << i;
        TestStep t;
        t.channel = ch_id.str();
        m_test_steps.push_back(t);
    }

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
    fpga_triggers["latency"] = "0"; // set the latency to zero for xADC sampling
    comm()->configure_fpga(fpga_triggers, fpga_clocks);

    // build the VMM config info for this step
    json vmm_config = m_base_vmm_config;
    json vmm_spi = vmm_config.at("vmm_spi");
    json vmm_globals = vmm_spi.at("global_registers");
    json vmm_channels = vmm_spi.at("channel_registers");

    // default all channels
    vector<vts::vmm::Channel> channels; 
    for(size_t i = 0; i < 64; i++)
    {
        json jch = {{"id",i},
                    {"sc",false},
                    {"sl",false},
                    {"sth",false},
                    {"st",false},
                    {"sm",false},
                    {"smx",false},
                    {"sd",0}};
        vmm::Channel ch;
        ch.load(jch);
        channels.push_back(ch);
    }
    vmm_spi["channel_registers"] = vts::vmm::channel_vec_to_json_config(channels).at("channel_registers");

    // set global VMM configuration
    vmm_globals["sm5"] = t.channel; // select the channel we want to measure from
    vmm_globals["sbmx"] = "ENABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "DISABLED";
    vmm_globals["sdt_dac"] = "200";
    vmm_globals["sdp_dac"] = "300";
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

    // start the initial xADC sampling, the rest will be done in process_event (if needed)
    comm()->sample_xadc(n_events_per_step() /*, int sampling_delay*/);

    // keep running until data processing has completed
    while(processing_events())
    {
        if(!processing_events()) break;
        continue;
    }
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
    string current_channel = t.channel;

    // decode the data packet
    vector<vts::decode::xadc::Sample> samples = vts::decode::xadc::decode(fragment->packet);
    for(const auto & sample : samples)
    {
        check_status(); // can do this inside or outside of this loop, here it will be on every single sample
        if(n_events_processed() >= n_events_per_step()) break;
        // process the samples (fill histograms/etc)

        // increment the counters since a single xADC sample is a single "event" (REQUIRED)
        event_processed();
    }

    // continue sampling events (loop will exit if we are already at the limit) (REQUIRED)
    comm()->sample_xadc(n_events_per_step()/*, int sampling_delay*/);
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
