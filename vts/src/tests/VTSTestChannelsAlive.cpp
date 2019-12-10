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

    m_test_data = m_test_config.at("test_data");
    // get the base configs for the fpga and VMM
    string config_dir = m_configuration_dirs.at("frontend").get<std::string>();
    string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    string vmm_file = m_test_data.at("base_config_global").get<std::string>();
    fpga_file = config_dir + "/" + fpga_file;
    vmm_file = config_dir + "/" + vmm_file;

    try
    {
        string n_cktp_str = m_test_data.at("n_cktp_per_cycle").get<std::string>();
        string cycle_time_str = m_test_data.at("time_per_cycle").get<std::string>();

        n_cktp_per_cycle = std::stoi(n_cktp_str);
        m_time_per_cycle = std::stoi(cycle_time_str);
    }
    catch(std::exception& e)
    {
        log->warn("{0} - Failed to load test parameters: {1}",__VTFUNC__,e.what());
        n_cktp_per_cycle = 50;
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

    // art dif
    h_art_diff = new TH1F("h_art_diff", "ART Diff", 100, 0, -1);
    h_bad_art = new TH1F("h_bad_art", "Bad ART Occurrences;VMM Channel;Entries", 64, 0, 64);
    h2_channel_art = new TH2F("h2_channel_art", "ART vs VMM Channel;VMM Channel;ART Address",64,0,64,64,0,64);
    h2_channel_art->SetDrawOption("colz");

    // initialize all counters
    m_bad_channels.clear();
    m_dead_channels.clear();
    m_bad_art_channels.clear();
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
            }
            else
            {
                m_retry_map.at(channel)++;
                redo = true;
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
    } // sample

    // increment the event counter to move forward in the test (REQUIRED)
    event_processed();
    // signal the event processing status to anyone listing (e.g. progress bar)
    check_status();

    return true;
}

bool VTSTestChannelsAlive::analyze()
{
    return true;
}

bool VTSTestChannelsAlive::analyze_test()
{
    int n_total_test_pulses_sent = n_cktp_per_cycle;

    for(int ichan = 0; ichan < 64; ichan++)
    {
        bool chan_marked_bad = false;

        int ibin = ichan+1;
        auto bc = h_channel_occ_total->GetBinContent(ibin);
        float eff = (bc / n_total_test_pulses_sent);
        h_channel_eff_total->SetBinContent(ibin, eff);

        // check the ART address
        int n_bad_art_for_chan = h_bad_art->GetBinContent(ibin);
        float bad_art_frac = (n_bad_art_for_chan*1.0) / bc;
        float art_eff = (1.0 - bad_art_frac);

        if(eff==0)
        {
            log->info("{0} - Dead VMM channel found: {1}",__VTFUNC__, ichan);
            chan_marked_bad = true;
            m_dead_channels.push_back(ichan);
        }
        else if(eff <= 0.75)
        {
            log->info("{0} - Low efficiency channel found: channel={1} eff={2}",__VTFUNC__,ichan,eff);
        }

        float art_threshold = 0.98;
        if((art_eff < art_threshold) && (eff>0))
        {
            log->info("{0} - ART for channel {1} is bad too often, marking channel as bad",__VTFUNC__,ichan);
            chan_marked_bad = true;
            m_bad_art_channels.push_back(ichan);
        }

        if(chan_marked_bad)
        {
            m_bad_channels.push_back(ichan);
        }
    }

    store(h_channel_occ_total);
    store(h_channel_eff_total);
    store(h_art_diff);
    store(h2_channel_art);
    store(h_bad_art);
    delete h_channel_occ_total;
    delete h_channel_eff_total;
    delete h_art_diff;
    delete h2_channel_art;
    delete h_bad_art;

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
            {"DEAD_CHANNELS",m_dead_channels},
            {"BAD_ART_CHANNELS",m_bad_art_channels}
        };
        jresults["REASON"] = jreason;
    }

    return jresults;
}

} // namespace vts