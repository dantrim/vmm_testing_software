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


bool VTSTestBaselines::initialize(const json& config)
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

        // histograms
        stringstream hname;
        stringstream hax;
        hname << "h_baselines_ch" << i;
        hax << "VMM Channel Baseline Samples;xADC Samples [mV];Entries";
        TH1F* h = new TH1F(hname.str().c_str(), hax.str().c_str(), 100, 0, -1);
        h->SetLineColor(kBlack);
        m_histos_baselines.push_back(h);
        m_bad_baselines.push_back(0);

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

    n_bad_channels = 0;

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

bool VTSTestBaselines::need_to_redo()
{
    if(get_current_state()>1)
    {
        int idx_step_current = get_current_state()-1;
        int idx_step_previous = (idx_step_current-1);
        TestStep t = m_test_steps.at(idx_step_previous);
        int channel = std::stoi(t.channel);
        auto h = m_histos_baselines.at(channel);
        float mean = h->GetMean();
        if(!(mean>0.0))
        {
            return true;
        }
    }
    return false;
}

void VTSTestBaselines::redo_last_step()
{
    int state_before_redo = (get_current_state());
    int state_after_redo = (get_current_state() -1);
    log->warn("{0} - Going back to previous state (state was={1}, now={2}) (channel was={3}, now={4})",__VTFUNC__, state_before_redo, state_after_redo, (state_before_redo-1), (state_after_redo-1));
    set_current_state( get_current_state() -1);
    return;
}

bool VTSTestBaselines::configure()
{
    bool redo = need_to_redo();
    if(redo)
    {
        log->warn("{0} - Re-doing last step",__VTFUNC__);
        redo_last_step();
    }
    
    TestStep t = m_test_steps.at(get_current_state() - 1);
    if(redo)
    {
        log->warn("{0} - Got step for channel {1}",__VTFUNC__, t.channel);
    }

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
    comm()->configure_vmm(vmm_config, /*perform reset*/ true);
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
        if(sample_mv>0.0)
            event_processed();
    }

    // continue sampling events (loop will exit if we are already at the limit) (REQUIRED)
    comm()->sample_xadc(n_events_per_step()/*, int sampling_delay*/);
    return true;
}


bool VTSTestBaselines::analyze()
{
    TestStep t = m_test_steps.at(get_current_state() - 1);
    int channel = std::stoi(t.channel);

    auto h = m_histos_baselines.at(channel);
    //store(h);
    float channel_baseline_mean = h->GetMean();
    bool lo_ok = (channel_baseline_mean >= LO_BASELINE_THRESHOLD);
    bool hi_ok = (channel_baseline_mean <= HI_BASELINE_THRESHOLD);
    bool going_to_retry = false;
    if(lo_ok && hi_ok)
    {
        m_bad_baselines.at(channel) = 0;
    }
    else
    {
        if(m_retry_map.at(channel) >= N_RETRY_MAX)
        {
            n_bad_channels++;
            m_bad_baselines.at(channel) = 1;
        }
        else
        {
            log->warn("{0} - Incrementing retry for channel {1} (mean was found to be {2} mV)",__VTFUNC__,channel, channel_baseline_mean);
            going_to_retry = true;
            m_retry_map.at(channel)++;
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
    return jresults;
}

} // namespace vts
