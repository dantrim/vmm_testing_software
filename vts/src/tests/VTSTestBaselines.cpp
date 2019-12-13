//vts
#include "communicator_frontend.h"
#include "tests/VTSTestBaselines.h"
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


//ROOT
#include "TH1F.h"

namespace vts
{


bool VTSTestBaselines::initialize(const json& /*config*/)
{
    m_test_data = m_test_config.at("test_data");
    m_finished_early = false;
    try
    {
        std::string exit_str = m_test_data.at("exit_on_fail").get<std::string>();
        if(exit_str == "YES")
        {
            m_exit_on_fail = true;
        }
        else
        {
            m_exit_on_fail = false;
        }
    }
    catch(std::exception& e)
    {
        m_exit_on_fail = false;
    }


    // get the base configs for the fpga and VMM
    //stringstream fpga_file;
    string config_dir = m_configuration_dirs.at("frontend").get<std::string>();
    string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    string vmm_file = m_test_data.at("base_config_global").get<std::string>();
    fpga_file = config_dir + "/" + fpga_file;
    vmm_file = config_dir + "/" + vmm_file;

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

        // histograms
        stringstream hname;
        stringstream hax;
        hname << "h_baselines_ch" << i;
        hax << "VMM Channel Baseline Samples;xADC Samples [mV];Entries";
        TH1F* h = new TH1F(hname.str().c_str(), hax.str().c_str(), 100, 0, -1);
        h->SetLineColor(kBlack);
        m_histos_baselines.push_back(h);

        m_retry_map[i] = 0;
    }
    stringstream hname;
    stringstream hax;
    hname << "h_vmm_channel_baseline_summary";
    hax << "VMM Channel Baseline Summary;VMM Channel;<Baseline> [mV]";
    h_baseline_summary = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_baseline_summary->SetLineColor(kBlack);

    hname.str("");
    hax.str("");
    hname << "h_vmm_channel_noise_summary";
    hax << "VMM Channel Noise Summary;VMM Channel;Noise [mV]";
    h_noise_summary = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_noise_summary->SetLineColor(kBlack);

    m_bad_baselines.clear();

    // initialize all counters
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestBaselines::load()
{
    set_current_state( get_current_state() + 1);
    return true;
}

bool VTSTestBaselines::need_to_redo_last_step()
{
    bool redo = false;
    m_current_retry = -1;
    if(get_current_state()>0) // states, or steps, start counting at 1
    {
        int idx_step_current = get_current_state()-1;
        int idx_step_previous = (idx_step_current-1);
        TestStep t = m_test_steps.at(get_current_state()-1);
        int channel = std::stoi(t.channel);
        auto h = m_histos_baselines.at(channel);
        float mean = h->GetMean();
        if(mean<10)
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
                set_n_events_for_test(m_test_steps.size() * n_events_per_step() + n_events_per_step());
            }
        }
    }
    return redo;
}

void VTSTestBaselines::redo_last_step()
{
    set_current_state(get_current_state()-1);
    return;
}

bool VTSTestBaselines::configure()
{
//    bool redo = need_to_redo_last_step();
//    if(redo)
//    {
//        log->debug("{0} - Re-doing last step",__VTFUNC__);
//        redo_last_step();
//    }
    
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
    vmm_spi["global_registers"] = vmm_globals;
    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration SPI string
    //comm()->configure_vmm(vmm_config, /*perform reset*/ true);
    comm()->configure_vmm(vmm_config, /*perform reset*/ false);
    return true;
}

bool VTSTestBaselines::run()
{
    // reset the event counters for this new test step
    reset_event_count();

    // start the initial xADC sampling, the rest will be done in process_event (if needed)
    comm()->sample_xadc(2.0 * n_events_per_step() /*, int sampling_delay*/);

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
    // return false to stop DAQ and move to next step in the testing
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        return false;
    }

    // get the configuration for thist test step
    TestStep t = m_test_steps.at(get_current_state() - 1);
    string current_channel = t.channel;
    int channel = std::stoi(current_channel);

    // decode the data packet
    vector<vts::decode::xadc::Sample> samples = vts::decode::xadc::decode(fragment->packet);
    for(const auto & sample : samples)
    {
        check_status(); // can do this inside or outside of this loop, here it will be on every single sample
        if(n_events_processed() >= n_events_per_step()) break;
        // process the samples (fill histograms/etc)

        float sample_mv = sample.sample();
        sample_mv = (sample_mv / pow(2,12)) * 1000.;
        m_histos_baselines.at(channel)->Fill(sample_mv);

        // increment the counters since a single xADC sample is a single "event" (REQUIRED)
        //if(sample_mv>0.0)
            event_processed();
    }

    // continue sampling events (loop will exit if we are already at the limit) (REQUIRED)
    comm()->sample_xadc(n_events_per_step()/*, int sampling_delay*/);
    return true;
}


bool VTSTestBaselines::analyze()
{
    bool redo = need_to_redo_last_step();
    if(redo)
    {
        redo_last_step();
        log->debug("{0} - Re-doing step {1}",__VTFUNC__,get_current_state());
    }
    if(redo) return true;

    TestStep t = m_test_steps.at(get_current_state() - 1);
    int channel = std::stoi(t.channel);


    auto h = m_histos_baselines.at(channel);
    float channel_baseline_mean = h->GetMean();
    bool lo_ok = (channel_baseline_mean >= LO_BASELINE_THRESHOLD);
    bool hi_ok = (channel_baseline_mean <= HI_BASELINE_THRESHOLD);
    bool baseline_within_bound = (lo_ok && hi_ok);
    bool going_to_retry = false;

    if(!baseline_within_bound)
    {
        if(!(m_current_retry>0))
        {
            m_bad_baselines.push_back(channel);

            // short-circuit
            if(m_exit_on_fail && m_bad_baselines.size()>2)
            {
                log->info("{0} - Finishing test early, more than 2 failed channels encountered",__VTFUNC__);
                set_current_state(m_test_steps.size());            
                m_finished_early = true;
            }
        }
        else
        {
            going_to_retry = true;
            log->info("{0} - Potential bad channel: {1} {2}",__VTFUNC__, channel, m_retry_map.at(channel));
        }
    }

    if(!going_to_retry)
    {
        float channel_baseline_noise = h->GetStdDev();
        m_channel_baseline_means.push_back(channel_baseline_mean);
        m_channel_noise.push_back(channel_baseline_noise);
    }

    return true;
}

bool VTSTestBaselines::analyze_test()
{
    for(size_t ichan = 0; ichan < m_channel_baseline_means.size(); ichan++)
    {
        h_baseline_summary->SetBinContent(ichan+1, m_channel_baseline_means.at(ichan));
        h_noise_summary->SetBinContent(ichan+1, m_channel_noise.at(ichan));
    }
    store(h_baseline_summary);
    store(h_noise_summary);
    return true;
}

bool VTSTestBaselines::finalize()
{
    for(size_t ih = 0 ; ih < m_histos_baselines.size(); ih++)
    {
        auto h = m_histos_baselines.at(ih);
        delete h;
    }

    return true;
}

json VTSTestBaselines::get_results()
{
    VTSTestResult result = VTSTestResult::TESTRESULTINVALID;
    size_t n_bad_channels = m_bad_baselines.size();
    if(n_bad_channels>2)
    {
        result = VTSTestResult::FAIL;
    }
    else if(n_bad_channels>0 && n_bad_channels<=2)
    {
        result = VTSTestResult::PASS;
    }
    else if(n_bad_channels==0)
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
            {"BAD_BASELINES",m_bad_baselines}
        };
        jresults["REASON"] = jreason;
    }

    return jresults;
}

} // namespace vts
