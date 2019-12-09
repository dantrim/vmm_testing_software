//vts
#include "communicator_frontend.h"
#include "tests/VTSTestChannelsAliveSingle.h"
#include "helpers.h"
#include "frontend_struct.h"
#include "daq_defs.h"
#include "vts_decode.h"

//std/stl
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

//logging
#include "spdlog/spdlog.h"

//ROOT
#include "TH1F.h"
#include "TH2F.h"

namespace vts
{


bool VTSTestChannelsAliveSingle::initialize(const json& config)
{
    stringstream msg;
    msg << "Initializing with config: " << config.dump();
    log->debug("{0} - {1}",__VTFUNC__,msg.str());

    m_test_data = m_test_config.at("test_data");
    // get the base configs for the fpga and VMM
    string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    string vmm_file = m_test_data.at("base_config_global").get<std::string>();

    n_cktp_per_cycle = 50;
    n_cycles = 1;
    m_time_per_cycle = 100;

    try
    {
        string n_cktp_str = m_test_data.at("n_cktp_per_cycle").get<std::string>();
        string n_cycles_str = m_test_data.at("n_cycles").get<std::string>();
        string cycle_time_str = m_test_data.at("time_per_cycle").get<std::string>();

        n_cktp_per_cycle = std::stoi(n_cktp_str);
        n_cycles = std::stoi(n_cycles_str);
        m_time_per_cycle = std::stoi(cycle_time_str);
    }
    catch(std::exception& e)
    {
        log->warn("{0} - Failed to load test parameters: {1}",__VTFUNC__,e.what());
        n_cktp_per_cycle = 50;
        n_cycles = 10;
        m_time_per_cycle = 50;
    }
    set_n_events_per_step(n_cktp_per_cycle);

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
    stringstream hname;
    stringstream hax;
    hname << "h_channel";
    hax << "Channel Hit Map;VMM Channel;Entries";
    h_channel = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    hname.str("");
    hax.str("");
    hname << "h2_pdo";
    hax << "Channel PDO;VMM Channel;PDO [counts]";
    h2_pdo = new TH2F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64, 50, 0, 1023);

    for(int i = 0; i < 64; i++)
    {
        TestStep t;
        t.channel = i;
        m_test_steps.push_back(t);
    } // i

    // initialize all counters
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestChannelsAliveSingle::load()
{
    set_current_state( get_current_state() + 1);
    return true;
}

bool VTSTestChannelsAliveSingle::configure()
{
    TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    stringstream cktp_max;
    int tmp = (n_cktp_per_cycle - 1);
    cktp_max << tmp;
    fpga_clocks["cktp_max_number"] = cktp_max.str();
    fpga_clocks["cktp_period"] = "1000";
    comm()->configure_fpga(fpga_triggers, fpga_clocks);

    // build the VMM config info for this step
    json vmm_config = m_base_vmm_config;
    json vmm_spi = vmm_config.at("vmm_spi");
    json vmm_globals = vmm_spi.at("global_registers");
    json vmm_channels = vmm_spi.at("channel_registers");

    // pulse all channels
    vector<vts::vmm::Channel> channels; 
    for(int i = 0; i < 64; i++)
    {
        bool pulse = (i==t.channel ? true : false);
        bool mask = (i==t.channel ? false : true);
        json jch = {{"id",i},
                    {"sc",false},
                    {"sl",false},
                    {"sth",false},
                    {"st",pulse},
                    {"sm",mask},
                    {"smx",false},
                    {"sd",0}};
        vmm::Channel ch;
        ch.load(jch);
        channels.push_back(ch);
    }
    vmm_spi["channel_registers"] = vts::vmm::channel_vec_to_json_config(channels).at("channel_registers");

    // set global VMM configuration
    vmm_globals["sm5"] = "0"; // monitor channel 0
    vmm_globals["sbmx"] = "ENABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "DISABLED";
//    vmm_globals["sdt_dac"] = "200"; // fix the threshold
//    vmm_globals["sdp_dac"] = t.dac_pulser;
//    vmm_globals["sg"] = "1.0";
    vmm_spi["global_registers"] = vmm_globals;
    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration SPI string
    comm()->configure_vmm(vmm_config, /*perform reset*/ false);
    return true;
}

bool VTSTestChannelsAliveSingle::run()
{
    // reset the event counters for this new test step
    reset_event_count();

    // start acq
    comm()->acq_toggle(true);

    auto start_time = std::chrono::system_clock::now();

    // keep running until data processing has completed
    while(processing_events())
    {
        if(!processing_events()) break;
        auto current_time = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if(diff >= m_time_per_cycle)
        {
            processing(false);
            break;
        }
    
        continue;
    }

    // stop acq now that we're done
    comm()->acq_toggle(false);
    return true;
}

bool VTSTestChannelsAliveSingle::process_event(vts::daq::DataFragment* fragment)
{
    // return false to stop DAQ and move to next step in the testing
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        return false;
    }

    // get the configuration for thist test step
    TestStep t = m_test_steps.at(get_current_state() - 1);
    int current_channel = t.channel;

    // decode the data packet
    vector<vts::decode::vmm::Sample> samples = vts::decode::vmm::decode(fragment->packet);
    for(const auto & sample : samples)
    {
        if(n_events_processed() >= n_events_per_step()) break;
        // process the samples (fill histograms/etc)
        h_channel->Fill(sample.channel());
        h2_pdo->Fill(sample.channel(), sample.pdo());
    }

    // increment the event counter to move forward in the test (REQUIRED)
    event_processed();
    // signal the event processing status to anyone listing (e.g. progress bar)
    check_status();
    return true;
}


bool VTSTestChannelsAliveSingle::analyze()
{
    TestStep t = m_test_steps.at(get_current_state() - 1);
    return true;
}

bool VTSTestChannelsAliveSingle::analyze_test()
{
    store(h_channel);
    store(h2_pdo);

    delete h_channel;
    delete h2_pdo;

    return true;
}

bool VTSTestChannelsAliveSingle::finalize()
{
    return true;
}

json VTSTestChannelsAliveSingle::get_results()
{
    json jresults = {
        {"RESULT",VTSTestResultToStr(VTSTestResult::SUCCESS)}
    };
    return jresults;
}

} // namespace vts
