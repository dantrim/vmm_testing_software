//vts
#include "communicator_frontend.h"
#include "tests/VTSTestChannelsAlivePos.h"
#include "helpers.h"
#include "frontend_struct.h"
#include "daq_defs.h"
#include "vts_decode.h"

//std/stl
#include <fstream>
#include <chrono>
#include <sstream>
#include <vector>
using namespace std;

//logging
#include "spdlog/spdlog.h"

//ROOT
#include "TH1F.h"

namespace vts
{


bool VTSTestChannelsAlivePos::initialize(const json& config)
{
    n_cktp_per_cycle = 50;
    n_cycles = 10;
    m_time_per_cycle = 100;

    stringstream msg;
    msg << "Initializing with config: " << config.dump();
    log->debug("{0} - {1}",__VTFUNC__,msg.str());

    m_test_data = m_test_config.at("test_data");
    // get the base configs for the fpga and VMM
    string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    string vmm_file = m_test_data.at("base_config_global").get<std::string>();
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
        m_time_per_cycle = 100;
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
    h_vec_channels_cycle.clear();
    for(int i = 0; i < n_cycles; i++)
    {
        TestStep t;
        t.cycle = i;
        m_test_steps.push_back(t);

        stringstream hname;
        stringstream hax;
        hname << "h_cycle_" << i << "_channels";
        hax << "VMM Channel Occupancy for Cycle " << i << ";VMM Channel;Entries";
        TH1F* h = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
        h->SetLineColor(kBlack);
        h_vec_channels_cycle.push_back(h);
    } // i

    stringstream hname;
    stringstream hax;
    hname << "h_channel_occ_total";
    hax << "VMM Channel Occupancy Over All Cycles;VMM Channel;Entries";
    h_channel_occ_total = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_channel_occ_total->SetLineColor(kBlack);
    hname.str(""); hax.str("");
    hname << "h_channel_eff_total";
    hax << "VMM Channel Efficiency Over All Cycles;VMM Channel;Efficiency";
    h_channel_eff_total = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_channel_eff_total->SetLineColor(kBlack);

    // initialize all counters
    m_dead_channels.clear();
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestChannelsAlivePos::load()
{
    set_current_state( get_current_state() + 1);
    return true;
}

bool VTSTestChannelsAlivePos::configure()
{
    comm()->configure_vmm(m_base_vmm_config, /*perform reset*/ true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    stringstream cktp_max;
    int tmp = (n_cktp_per_cycle - 1);
    cktp_max << tmp;
    fpga_clocks["cktp_max_number"] = cktp_max.str(); // the firmware adds 1
    fpga_clocks["cktp_period"] = "5000";
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
    stringstream chan_str;
    chan_str << "0";
    vmm_globals["sm5"] = chan_str.str(); // monitor channel 0
    vmm_globals["sbmx"] = "ENABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "DISABLED";
    vmm_spi["global_registers"] = vmm_globals;
    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration SPI string
    comm()->configure_vmm(vmm_config, /*perform reset*/ false);
    return true;
}

bool VTSTestChannelsAlivePos::run()
{
    // reset the event counters for this new test step
    reset_event_count();

    // start acq
    m_start_time = std::chrono::system_clock::now();
    comm()->acq_toggle(true);

    // keep running until data processing has completed
    while(processing_events())
    {
        if(!processing_events()) break;
        auto current_time = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - m_start_time).count();
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

bool VTSTestChannelsAlivePos::process_event(vts::daq::DataFragment* fragment)
{

    // return false to stop DAQ and move to next step in the testing
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        return false;
    }

    // get the configuration for thist test step
    TestStep t = m_test_steps.at(get_current_state() - 1);
    int cycle = t.cycle;

    // decode the data packet
    vector<vts::decode::vmm::Sample> samples = vts::decode::vmm::decode(fragment->packet);

    for(const auto & sample : samples)
    {
        h_vec_channels_cycle.at(cycle)->Fill(sample.channel());
    } // sample

    // increment the event counter to move forward in the test (REQUIRED)
    event_processed();
    // signal the event processing status to anyone listing (e.g. progress bar)
    check_status();

    return true;
}

bool VTSTestChannelsAlivePos::analyze()
{
    return true;
}

bool VTSTestChannelsAlivePos::analyze_test()
{
    int n_total_test_pulses_sent = (m_test_steps.size() * n_cktp_per_cycle);
    for(const auto & h : h_vec_channels_cycle)
    {
        for(int ichan = 0; ichan < 64; ichan++)
        {
            int ibin = ichan+1;
            auto bc = h->GetBinContent(ibin);
            auto current = h_channel_occ_total->GetBinContent(ibin);
            h_channel_occ_total->SetBinContent(ibin, current+bc);
        } // ichan
        store(h);
        delete h;
    } // h

    for(int ichan = 0; ichan < 64; ichan++)
    {
        int ibin = ichan+1;
        auto bc = h_channel_occ_total->GetBinContent(ibin);
        float eff = (bc / n_total_test_pulses_sent);
        h_channel_eff_total->SetBinContent(ibin, eff);

        if(eff==0)
        {
            log->info("{0} - Dead VMM channel found: {1}",__VTFUNC__, ichan);
            m_dead_channels.push_back(ichan);
        }
        else if(eff <= 0.75)
        {
            log->info("{0} - Low efficiency channel found: channel={1} eff={2}",__VTFUNC__,ichan,eff);
        }
    }

    store(h_channel_occ_total);
    store(h_channel_eff_total);
    delete h_channel_occ_total;
    delete h_channel_eff_total;

    return true;
}

bool VTSTestChannelsAlivePos::finalize()
{
    return true;
}

json VTSTestChannelsAlivePos::get_results()
{
    VTSTestResult result = VTSTestResult::TESTRESULTINVALID;
    size_t n_dead = m_dead_channels.size();
    if(n_dead>2)
    {
        result = VTSTestResult::FAIL;
    }
    else if(n_dead>0 && n_dead<=2)
    {
        result = VTSTestResult::PASS;
    }
    else if(n_dead==0)
    {
        result = VTSTestResult::SUCCESS;
    }

    json jresults = {
        {"RESULT",VTSTestResultToStr(result)}
    };
    return jresults;
}

} // namespace vts
