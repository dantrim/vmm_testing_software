//vts
#include "communicator_frontend.h"
#include "tests/VTSTestChannelsAlive.h"
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
#include "TH2F.h"

namespace vts
{


bool VTSTestChannelsAlive::initialize(const json& /*config*/)
{
    n_cktp_per_cycle = 50;
    m_time_per_cycle = 100;
    m_is_positive = false;
    m_exit_on_fail = false;
    m_finished_early = false;

    m_test_data = m_test_config.at("test_data");
    // get the base configs for the fpga and VMM
    string config_dir = m_configuration_dirs.at("frontend").get<std::string>();
    string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    string vmm_file = m_test_data.at("base_config_global").get<std::string>();
    fpga_file = config_dir + "/" + fpga_file;
    vmm_file = config_dir + "/" + vmm_file;

    string sub = "pos";
    m_is_positive = !(vmm_file.find(sub) == std::string::npos);

    try
    {
        string n_cktp_str = m_test_data.at("n_cktp_per_cycle").get<std::string>();
        string cycle_time_str = m_test_data.at("time_per_cycle").get<std::string>();
        string exit_str = m_test_data.at("exit_on_fail").get<std::string>();
        if(exit_str == "YES")
        {
            m_exit_on_fail = true;
        }
        else
        {
            m_exit_on_fail = false;
        }

        n_cktp_per_cycle = std::stoi(n_cktp_str);
        m_time_per_cycle = std::stoi(cycle_time_str);
    }
    catch(std::exception& e)
    {
        log->warn("{0} - Failed to load test parameters: {1}",__VTFUNC__,e.what());
        n_cktp_per_cycle = 50;
        m_time_per_cycle = 100;
        m_exit_on_fail = false;
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
    for(int i = 0; i < 64; i++)
    {
        TestStep t;
        t.channel = i;
        m_test_steps.push_back(t);
        m_retry_map[i] = 0;
    } // i

    stringstream hname;
    stringstream hax;
    hname << "h_channel_occ_total";
    hax << "VMM Channel Occupancy;VMM Channel;Entries";
    h_channel_occ_total = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_channel_occ_total->SetLineColor(kBlack);
    hname.str(""); hax.str("");
    hname << "h_channel_eff_total";
    hax << "VMM Channel Efficiency;VMM Channel;Efficiency";
    h_channel_eff_total = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_channel_eff_total->SetLineColor(kBlack);

    h2_channel_pdo = new TH2F("h2_channel_pdo", "Channel PDO; VMM Channel;PDO [counts]", 64,0,64,100,0,1023);

    // art dif
    h_art_diff = new TH1F("h_art_diff", "ART Diff", 100, 0, -1);
    h_bad_art = new TH1F("h_bad_art", "Bad ART Occurrences;VMM Channel;Entries", 64, 0, 64);
    h2_channel_art = new TH2F("h2_channel_art", "ART vs VMM Channel;VMM Channel;ART Address",64,0,64,64,0,64);
    h2_channel_art->SetDrawOption("colz");

    // initialize all counters
    m_bad_channels.clear();
    m_dead_channels.clear();
    m_bad_art_channels.clear();
    m_bad_pdo_channels.clear();
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestChannelsAlive::load()
{
    set_current_state( get_current_state() + 1);
    return true;
}

bool VTSTestChannelsAlive::need_to_redo_last_step()
{
    bool redo = false;
    m_current_retry = -1;
    if(get_current_state()>1) // states, or steps, start counting at 1
    {
        int idx_step_curr = get_current_state()-1;
        int idx_step_prev = idx_step_curr-1;
        TestStep t = m_test_steps.at(idx_step_prev);
        int channel = t.channel;
        int bin_no = (channel+1);
        int bc = h_channel_occ_total->GetBinContent(bin_no);
        if(bc==0)
        {
            if(m_retry_map.at(channel)>=N_RETRY_MAX)
            {
                redo = false;
                m_current_retry = -1;
            }
            else
            {
                redo = true;
                m_retry_map.at(channel)++;
                m_current_retry = channel;
            }
        }
    }
    return redo;
}

void VTSTestChannelsAlive::redo_last_step()
{
    set_current_state(get_current_state()-1);
}

bool VTSTestChannelsAlive::configure()
{
    bool do_redo = need_to_redo_last_step();
    if(do_redo)
    {
        redo_last_step();
    }
    TestStep t = m_test_steps.at(get_current_state() - 1);
    if(do_redo)
    {
        log->debug("{0} - Re-doing step {1}",__VTFUNC__, get_current_state());
    }

    //comm()->configure_vmm(m_base_vmm_config, /*perform reset*/ true);
    //std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    stringstream cktp_max;
    int tmp = (n_cktp_per_cycle - 1);
    tmp = -1;
    cktp_max << tmp;
    fpga_clocks["cktp_max_number"] = cktp_max.str(); // the firmware adds 1
    //fpga_clocks["cktp_period"] = "5000";
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
        bool mask = (i!=t.channel);
        json jch = {{"id",i},
                    {"sc",false},
                    {"sl",false},
                    {"sth",m_is_positive},
                    {"st",true},
                    {"sm",mask},
                    {"smx",false},
                    {"sd",0}};
        vmm::Channel ch;
        ch.load(jch);
        channels.push_back(ch);
    }
    vmm_spi["channel_registers"] = vts::vmm::channel_vec_to_json_config(channels).at("channel_registers");

    // set global VMM configuration
    vmm_spi["global_registers"] = vmm_globals;
    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration SPI string
    comm()->configure_vmm(vmm_config, /*perform reset*/ false);
    return true;
}

bool VTSTestChannelsAlive::run()
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

bool VTSTestChannelsAlive::process_event(vts::daq::DataFragment* fragment)
{

    // return false to stop DAQ and move to next step in the testing
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        return false;
    }

    // get the configuration for thist test step
    //TestStep t = m_test_steps.at(get_current_state() - 1);

    // decode the data packet
    vector<vts::decode::vmm::Sample> samples = vts::decode::vmm::decode(fragment->packet);

    for(const auto & sample : samples)
    {
        auto art = sample.header().art_address();
        int diff = art - sample.channel();
        if(!(diff == 0))
        {
            h_bad_art->Fill(sample.channel());
        }
        h_channel_occ_total->Fill(sample.channel());
        h_art_diff->Fill(diff);
        h2_channel_art->Fill(sample.channel(), art);
        h2_channel_pdo->Fill(sample.channel(), sample.pdo());
    } // sample

    // increment the event counter to move forward in the test (REQUIRED)
    event_processed();
    // signal the event processing status to anyone listing (e.g. progress bar)
    check_status();

    return true;
}

bool VTSTestChannelsAlive::analyze()
{
    int n_total_test_pulses_sent = n_cktp_per_cycle;
    TestStep t = m_test_steps.at(get_current_state() - 1);
    int current_channel = t.channel;
//    bool chan_marked_bad = false;

    int ibin = (current_channel+1);
    auto bc = h_channel_occ_total->GetBinContent(ibin);
    float eff = (bc/n_total_test_pulses_sent);

    int n_bad_art_for_chan = h_bad_art->GetBinContent(ibin);
    float bad_art_frac = (n_bad_art_for_chan*1.0)/bc;
    float art_eff = (1.0 - bad_art_frac);

    bool bad_eff = false;
    bool bad_art = false;
    bool bad_pdo = false;

    if(eff==0)
    {
        //log->info("{0} - Dead VMM channel found: {1}",__VTFUNC__, current_channel);
        bad_eff = true;
    }
    //else if(eff <= 0.75)
    //{
    //    //log->info("{0} - Low efficiency channel found: channel = {1}, eff = {2}",__VTFUNC__, current_channel, eff);
    //}

    float art_threshold = 0.98;
    if((art_eff < art_threshold) && (eff>0))
    {
        bad_art = true;
    }

    // check PDO
    stringstream hname;
    hname << "hpdo_" << current_channel;
    TH1F* hpdo = (TH1F*)h2_channel_pdo->ProjectionY(hname.str().c_str(), ibin, ibin);
    float mean_pdo = hpdo->GetMean();
    delete hpdo;

    bool lo_pdo_ok = (mean_pdo >= LO_PDO_THRESHOLD);
    bool hi_pdo_ok = (mean_pdo <= HI_PDO_THRESHOLD);
    bool pdo_ok = (lo_pdo_ok && hi_pdo_ok);
    if(!(pdo_ok))
    {
        bad_pdo = true;
    }

    bool going_to_retry = false;
    bool bad_chan = (bad_eff || bad_art || bad_pdo);
    if(bad_chan)
    {
        if(!(m_current_retry>0))
        {
            m_bad_channels.push_back(current_channel);
            if(bad_eff)
                m_dead_channels.push_back(current_channel);
            if(bad_art)
                m_bad_art_channels.push_back(current_channel);
            if(bad_pdo)
                m_bad_pdo_channels.push_back(current_channel);

            if(m_exit_on_fail && m_bad_channels.size()>2)
            {
                log->info("{0} - Finishing test early, more than 2 failed channels encountered",__VTFUNC__);
                set_current_state(m_test_steps.size());
                m_finished_early = true;
            }
        } 
        else
        {
            going_to_retry = true;
        }
    } // bad_chan

    if(!going_to_retry)
    {
        h_channel_eff_total->SetBinContent(ibin, eff);
    }
    

    return true;
}

bool VTSTestChannelsAlive::analyze_test()
{
//    int n_total_test_pulses_sent = n_cktp_per_cycle;
//
//    for(int ichan = 0; ichan < 64; ichan++)
//    {
//        bool chan_marked_bad = false;
//
//        int ibin = ichan+1;
//        auto bc = h_channel_occ_total->GetBinContent(ibin);
//        float eff = (bc / n_total_test_pulses_sent);
//        h_channel_eff_total->SetBinContent(ibin, eff);
//
//        // check the ART address
//        int n_bad_art_for_chan = h_bad_art->GetBinContent(ibin);
//        float bad_art_frac = (n_bad_art_for_chan*1.0) / bc;
//        float art_eff = (1.0 - bad_art_frac);
//
//        if(eff==0)
//        {
//            log->info("{0} - Dead VMM channel found: {1}",__VTFUNC__, ichan);
//            chan_marked_bad = true;
//            m_dead_channels.push_back(ichan);
//        }
//        else if(eff <= 0.75)
//        {
//            log->info("{0} - Low efficiency channel found: channel={1} eff={2}",__VTFUNC__,ichan,eff);
//        }
//
//        float art_threshold = 0.98;
//        if((art_eff < art_threshold) && (eff>0))
//        {
//            log->info("{0} - ART for channel {1} is bad too often, marking channel as bad",__VTFUNC__,ichan);
//            chan_marked_bad = true;
//            m_bad_art_channels.push_back(ichan);
//        }
//
//        // check the PDO values
//        stringstream hname;
//        hname << "hpdo_" << ichan;
//        int bin_number = (ichan+1);
//        TH1F* hpdo = (TH1F*)h2_channel_pdo->ProjectionY(hname.str().c_str(), bin_number, bin_number);
//        float mean_pdo = hpdo->GetMean();
//        delete hpdo;
//        bool lo_pdo_ok = (mean_pdo >= LO_PDO_THRESHOLD);
//        bool hi_pdo_ok = (mean_pdo <= HI_PDO_THRESHOLD);
//        bool pdo_within_bounds = (lo_pdo_ok && hi_pdo_ok);
//        if(!pdo_within_bounds)
//        {
//            m_bad_pdo_channels.push_back(ichan);
//            chan_marked_bad = true;
//        }
//
//        if(chan_marked_bad)
//        {
//            m_bad_channels.push_back(ichan);
//        }
//    }

    store(h_channel_occ_total);
    store(h_channel_eff_total);
    store(h_art_diff);
    store(h2_channel_art);
    store(h_bad_art);
    store(h2_channel_pdo);
    delete h_channel_occ_total;
    delete h_channel_eff_total;
    delete h_art_diff;
    delete h2_channel_art;
    delete h_bad_art;
    delete h2_channel_pdo;

    return true;
}

bool VTSTestChannelsAlive::finalize()
{
    return true;
}

json VTSTestChannelsAlive::get_results()
{
    VTSTestResult result = VTSTestResult::TESTRESULTINVALID;
    size_t n_dead = m_bad_channels.size();
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

    if(result != VTSTestResult::SUCCESS)
    {
        json jreason = {
            {"FINISHED_EARLY",m_finished_early},
            {"DEAD_CHANNELS",m_dead_channels},
            {"BAD_ART_CHANNELS",m_bad_art_channels},
            {"BAD_PDO_CHANNELS",m_bad_pdo_channels}
        };
        jresults["REASON"] = jreason;
    }

    return jresults;
}

} // namespace vts
