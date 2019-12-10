//vts
#include "communicator_frontend.h"
#include "tests/VTSTestConfigurableVMM.h"
#include "helpers.h"
#include "frontend_struct.h"
#include "daq_defs.h"
#include "vts_decode.h"

//std/stl
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h> // rand
using namespace std;

//logging
#include "spdlog/spdlog.h"

//ROOT
#include "TH1F.h"

namespace vts
{


bool VTSTestConfigurableVMM::initialize(const json& /*config*/)
{
    m_test_data = m_test_config.at("test_data");
    // get the base configs for the fpga and VMM
    string config_dir = m_configuration_dirs.at("frontend").get<std::string>();
    string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    string vmm_file = m_test_data.at("base_config_global").get<std::string>();
    fpga_file = config_dir + "/" + fpga_file;
    vmm_file = config_dir + "/" + vmm_file;
    //string fpga_file = m_test_data.at("base_config_fpga").get<std::string>();
    //string vmm_file = m_test_data.at("base_config_global").get<std::string>();

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
    vector<int> channels;
    int n_channels_to_sample = 10;
    for(size_t i = 0; i < 2; i++)
    {
        bool configured = (i==0 ? false : true);
        stringstream hname;
        stringstream hax;
        for(int j = 0; j < n_channels_to_sample; j++)
        {

            int chan = -1;
            if(i==0)
            {
                // get a random channel
                chan = rand() % 63;
                auto it = std::find(channels.begin(), channels.end(), chan);
                while(it != channels.end())
                {
                    chan = rand() % 63;
                    it = std::find(channels.begin(), channels.end(), chan);
                }
                map_chan_to_idx[chan] = j;
                channels.push_back(chan);
            }
            else
            {
                chan = channels.at(j);
            }
            
            ch_id.str("");
            ch_id << chan;
            TestStep t;
            t.is_configured = configured;
            t.channel = ch_id.str();
            m_test_steps.push_back(t);


            // histos
            hname.str("");
            hax.str("");
            hname << "h_baselines_" << (configured ? "" : "NOT") << "configured_ch" << ch_id.str();
            hax << "Channel " << ch_id.str() << " " << (configured ? "" : "NOT") << " configured;xADC Samples [mV];Entries";
            TH1F* h = new TH1F(hname.str().c_str(), hax.str().c_str(), 100, 0, -1);
            h->SetLineColor(kBlack);
            if(configured)
            {
                m_histos_baselines_configured.push_back(h);
            }
            else
            {
                m_histos_baselines_not_configured.push_back(h);
            }
        } // j
    } // i
    stringstream hname;
    stringstream hax;
    hname << "h_noise_ratio";
    hax << "VMM Channel Noise Ratio;VMM Channel;Noise Not Configured / Noise Configured";
    m_histo_noise_ratio = new TH1F(hname.str().c_str(), hax.str().c_str(), n_channels_to_sample, 0, n_channels_to_sample);

    // initialize all counters
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestConfigurableVMM::load()
{
    set_current_state( get_current_state() + 1);
    return true;
}

bool VTSTestConfigurableVMM::configure()
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

    bool do_reset = (t.is_configured ? false : true);
    // send configuration SPI string
    comm()->configure_vmm(vmm_config, /*perform reset*/ do_reset);
    return true;
}

bool VTSTestConfigurableVMM::run()
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

bool VTSTestConfigurableVMM::process_event(vts::daq::DataFragment* fragment)
{
    // return false to stop DAQ and move to next step in the testing
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        return false;
    }

    // get the configuration for thist test step
    TestStep t = m_test_steps.at(get_current_state() - 1);
    bool should_be_configured = t.is_configured;
    string current_channel = t.channel;
    int channel = std::stoi(current_channel);

    // decode the data packet
    vector<vts::decode::xadc::Sample> samples = vts::decode::xadc::decode(fragment->packet);
    for(const auto & sample : samples)
    {
        check_status(); // can do this inside or outside of this loop, here it will be on every single sample
        if(n_events_processed() >= n_events_per_step()) break;
        // process the samples (fill histograms/etc)

        float sample_mv = sample.sample() * 1.0;
        sample_mv = (sample_mv / pow(2,12)) * 1000.;
        if(should_be_configured)
        {
            m_histos_baselines_configured.at(map_chan_to_idx[channel])->Fill(sample_mv);
        }
        else
        {
            m_histos_baselines_not_configured.at(map_chan_to_idx[channel])->Fill(sample_mv);
        }

        // increment the counters since a single xADC sample is a single "event" (REQUIRED)
        event_processed();
    }

    // continue sampling events (loop will exit if we are already at the limit) (REQUIRED)
    comm()->sample_xadc(n_events_per_step()/*, int sampling_delay*/);
    return true;
}


bool VTSTestConfigurableVMM::analyze()
{
    TestStep t = m_test_steps.at(get_current_state() - 1);
    return true;
}

bool VTSTestConfigurableVMM::analyze_test()
{
    size_t n_chan_configured = 0;
    for(size_t i = 0; i < m_histos_baselines_not_configured.size(); i++)
    {
        auto hnot = m_histos_baselines_not_configured.at(i);
        auto h = m_histos_baselines_configured.at(i);

        float noise_not_configured = hnot->GetStdDev();
        float noise_configured = h->GetStdDev();
        float noise_ratio = (noise_not_configured / noise_configured);
        m_histo_noise_ratio->SetBinContent(i+1, noise_ratio);

        if(noise_ratio > 100)
        {
            n_chan_configured++;
        }

        delete hnot;
        delete h;
    }
    store(m_histo_noise_ratio);
    delete m_histo_noise_ratio;

    // if any of the channels was configured, consider the VMM to be able to be
    // configured
    if(n_chan_configured > 0)
    {
        m_vmm_configurable = true;
    }
    else
    {
        m_vmm_configurable = false;
    }

    m_histos_baselines_not_configured.clear();
    m_histos_baselines_configured.clear();

    return true;
}

bool VTSTestConfigurableVMM::finalize()
{
    return true;
}

json VTSTestConfigurableVMM::get_results()
{
    VTSTestResult result = (m_vmm_configurable ? VTSTestResult::SUCCESS : VTSTestResult::FAIL);
    json jresults = {
        {"RESULT",VTSTestResultToStr(result)}
    };
    return jresults;
}

} // namespace vts
