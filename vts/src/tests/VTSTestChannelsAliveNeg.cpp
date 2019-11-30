//vts
#include "communicator_frontend.h"
#include "tests/VTSTestChannelsAliveNeg.h"
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


bool VTSTestChannelsAliveNeg::initialize(const json& config)
{
    n_times = 0;

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
    for(size_t i = 0; i < 1; i++)
    {
        TestStep t;
        t.cycle = i;
        m_test_steps.push_back(t);
    } // i

//    for(size_t i = 0; i < 1; i++)
//    {
//        TestStep t;
//        t.channel = i;
//        m_test_steps.push_back(t);
//    }

    h_over_count = new TH1F("h_over_count", ";VMM Channel;Over Count", 64, 0, 64);

    // initialize histograms
    m_dead_channels.clear();

    stringstream hname;
    stringstream hax;
    hname << "h_channel_occupancy";
    hax << "VMM Channel Occupancy;VMM Channel;Entries";
    h_channel_occupancy = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_channel_occupancy->SetLineColor(kBlack);

    hname.str("");
    hax.str("");
    hname << "h_channel_eff";
    hax << "VMM Channel Efficiency;VMM Channel;Fracion";
    h_channel_eff = new TH1F(hname.str().c_str(), hax.str().c_str(), 64, 0, 64);
    h_channel_eff->SetLineColor(kBlack);
    h_channel_eff->SetMaximum(3);
    h_channel_eff->SetMinimum(0);

    // initialize all counters
    set_current_state(0);
    set_n_states(m_test_steps.size());
    set_n_events_for_test(m_test_steps.size() * n_events_per_step());
    return true;
}

bool VTSTestChannelsAliveNeg::load()
{
    set_current_state( get_current_state() + 1);
    return true;
}

bool VTSTestChannelsAliveNeg::configure()
{
    comm()->configure_vmm(m_base_vmm_config, /*perform reset*/ true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    //fpga_clocks["cktp_max_number"] = "1";
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
    //vmm_globals["sg"] = "1.0";
    //vmm_globals["sdp_dac"] = "300";
    //vmm_globals["sdt_dac"] = "200";
    vmm_spi["global_registers"] = vmm_globals;
    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration SPI string
    comm()->configure_vmm(vmm_config, /*perform reset*/ false);
    return true;
}

bool VTSTestChannelsAliveNeg::run()
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

bool VTSTestChannelsAliveNeg::process_event(vts::daq::DataFragment* fragment)
{
    n_times++;
    int n_chans_at_peak = 0;
    for(int i = 0; i < 64; i++)
    {
        float bc = h_channel_occupancy->GetBinContent(i+1);
        if(bc>=n_events_per_step())
        {
            n_chans_at_peak++;
        }
    }
    //log->critical("{0} - N chans at peak = {1}",__VTFUNC__,n_chans_at_peak);
    bool stop = (n_chans_at_peak==64);
    float max_over = -1.0;
    for(int i = 0; i < 64; i++)
    {
        auto bc = h_over_count->GetBinContent(i+1);
        if(bc>max_over) max_over = bc;
    }
    if(max_over > 1.0 * n_events_per_step())
    {
        stop = true;
    }

    if(stop)
    {
        return false;
    }

    //// return false to stop DAQ and move to next step in the testing
    //if((n_events_processed() >= n_events_per_step()) || !processing_events())
    //{
    //    return false;
    //}

    // get the configuration for thist test step
    TestStep t = m_test_steps.at(get_current_state() - 1);
    int cycle = t.cycle;

    // decode the data packet
    vector<vts::decode::vmm::Sample> samples = vts::decode::vmm::decode(fragment->packet);
    //log->critical("{0} - Received event with {1} samples",__VTFUNC__,samples.size());
    for(int i = 0; i < 64; i++)
    {
        if(h_channel_occupancy->GetBinContent(i+1)>=n_events_per_step())
        {
            h_over_count->Fill(i);
        }
    }
    for(const auto & sample : samples)
    {
        if(stop) break;
        //if(n_events_processed() >= n_events_per_step()) break;
        // process the samples (fill histograms/etc)

        //if(sample.channel()==4 || sample.channel()==19 || sample.channel()==24) continue;
        h_channel_occupancy->Fill(sample.channel()+0.1);
    }

    // increment the event counter to move forward in the test (REQUIRED)
    event_processed();
    // signal the event processing status to anyone listing (e.g. progress bar)
    check_status();
    return true;
}


bool VTSTestChannelsAliveNeg::analyze()
{
    //TestStep t = m_test_steps.at(get_current_state() - 1);
    return true;
}

bool VTSTestChannelsAliveNeg::analyze_test()
{
    float expected_number = n_events_for_test() * 1.0;
    for(size_t ichan = 0; ichan < 64; ichan++)
    {
        float eff = (h_channel_occupancy->GetBinContent(ichan+1) / expected_number);
        if(eff == 0.0)
        {
            m_dead_channels.push_back(ichan);
        }
        h_channel_eff->SetBinContent(ichan+1, eff);
    } // ichan
    store(h_channel_eff);
    store(h_channel_occupancy);

    delete h_channel_eff;
    delete h_channel_occupancy;

    store(h_over_count);
    delete h_over_count;


    return true;
}

bool VTSTestChannelsAliveNeg::finalize()
{
    return true;
}

json VTSTestChannelsAliveNeg::get_results()
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
