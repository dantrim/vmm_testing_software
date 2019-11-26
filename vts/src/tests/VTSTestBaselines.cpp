//vts
#include "communicator_frontend.h" // for sending commands to the frontend
#include "tests/VTSTestBaselines.h"
#include "helpers.h"
#include "frontend_struct.h"
#include "daq_defs.h"
//decoding
#include "vts_decode.h"

//std/stl
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <math.h> // pow
using namespace std;

//logging
#include "spdlog/spdlog.h"

//ROOT
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TGraphErrors.h"

namespace vts
{


bool VTSTestBaselines::initialize(const json& config)
{

    stringstream msg;
    msg << "Initializing with config: " << config.dump();
    log->info("{0} - {1}",__VTFUNC__,msg.str());

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

    m_channel_histos.clear();
    m_channel_baseline_means.clear();

    m_test_steps.clear();
    stringstream ch;
    for(int i = 0; i < 64; i++)
    {
        ch.str("");
        ch << i;
        TestStep t;
        t.channel = ch.str();
        m_test_steps.push_back(t);

        // histogram
        stringstream hname;
        stringstream axis;
        hname << "baseline_samples_ch" << t.channel;
        axis << ";xADC samples [counts];Entries";
        TH1F* h = new TH1F(hname.str().c_str(), axis.str().c_str(),100,0,-1);
        h->SetLineColor(kBlack);
        m_channel_histos.push_back(h);
        m_channel_baseline_means.push_back(0.0);
        m_channel_baseline_errors.push_back(0.0);
    }
    
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

bool VTSTestBaselines::configure()
{
    //reset_vmm();
    TestStep t = m_test_steps.at(get_current_state() - 1);

    // configure the fpga
    json fpga_config = m_base_fpga_config;
    json fpga_registers = fpga_config.at("fpga_registers");
    json fpga_triggers = fpga_registers.at("trigger");
    json fpga_clocks = fpga_registers.at("clocks");
    fpga_triggers["latency"] = "0";

    comm()->configure_fpga(fpga_triggers, fpga_clocks);

    // build the VMM config info for this step
    json vmm_config = m_base_vmm_config;
    json vmm_spi = vmm_config.at("vmm_spi");
    json vmm_globals = vmm_spi.at("global_registers");
    json vmm_channels = vmm_spi.at("channel_registers");

    vector<vts::vmm::Channel> channels; 
    for(size_t i = 0; i < 64; i++)
    {
        bool st = false;
        bool sm =  false; //(i==14) ? false : true;
        bool smx = false; //(i==14) ? true : false;
        json jch = {{"id",i},{"sc",false},{"sl",false},{"sth",false},{"st",st},{"sm",sm},{"smx",smx},{"sd",0}};
        vmm::Channel ch; ch.load(jch);
        channels.push_back(ch);
    }
    auto chvec = vts::vmm::channel_vec_to_json_config(channels); 
    auto channel_registers = chvec.at("channel_registers");
    vmm_spi["channel_registers"] = channel_registers;

    vmm_globals["sm5"] = t.channel;
    vmm_globals["sbmx"] = "ENABLED";
    vmm_globals["scmx"] = "ENABLED";
    vmm_globals["sbfm"] = "DISABLED";
    vmm_globals["sdt_dac"] = "100";
    vmm_globals["sdp_dac"] = "400";
    vmm_globals["sg"] = "1.0";
    vmm_spi["global_registers"] = vmm_globals;

    vmm_config["vmm_spi"] = vmm_spi;

    // send configuration
    comm()->configure_vmm(vmm_config, false);

    return true;
}

bool VTSTestBaselines::run()
{
    // reset the event counters for this new run
    reset_event_count();

    comm()->sample_xadc(n_events_per_step(),500);

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
    if((n_events_processed() >= n_events_per_step()) || !processing_events())
    {
        //log->critical("{0} - Event count reached ({1},{2})",__VTFUNC__, n_events_processed(), processing_events());
        return false;
    }
    TestStep t = m_test_steps.at(get_current_state() - 1);
    string current_channel = t.channel;
    int channel = std::stoi(current_channel);

    vector<vts::decode::xadc::Sample> samples = vts::decode::xadc::decode(fragment->packet);
    for(const auto & sample : samples)
    {
        check_status();
        if(n_events_processed() >= n_events_per_step()) break;
        m_channel_histos.at(channel)->Fill(sample.sample());
        event_processed();
    }
    comm()->sample_xadc(n_events_per_step(), 500);
    return true;
}


bool VTSTestBaselines::analyze()
{
    TestStep t = m_test_steps.at(get_current_state() - 1);
    int channel = std::stoi(t.channel);

    auto h = m_channel_histos.at(channel);
    float mean_baseline_counts = h->GetMean();
    float mean_baseline_mv = (mean_baseline_counts / pow(2,12)) * 1000.;
    float mean_baseline_counts_err = h->GetStdDev();
    float mean_baseline_mv_err = (mean_baseline_counts_err / pow(2,12)) * 1000.;
    m_channel_baseline_means.at(channel) = mean_baseline_mv;
    m_channel_baseline_errors.at(channel) = mean_baseline_mv_err;

//    stringstream msg;
//    msg << "Average xADC sampling for channel " << channel << " = " << mean_baseline_mv << " +/- " << mean_baseline_mv_err << " [mV] (" <<  mean_baseline_counts << " +/- " << mean_baseline_counts_err << " [counts])";
//    log->info("{0} - {1}",__VTFUNC__,msg.str());

    return true;
}

bool VTSTestBaselines::analyze_test()
{
    TH1F* h = new TH1F("channel_baselines", ";VMM Channel;Baselines [mV]", 64,0,64);
    h->SetMarkerStyle(20);
    h->SetMarkerColor(kBlack);

    float max_y = -1.0;
    float min_y = 1e3;
    for(size_t i = 0; i < m_channel_baseline_means.size(); i++)
    {
        int ibin = i+1;
        float val = m_channel_baseline_means.at(i);
        if(val < min_y) min_y = val;
        if(val > max_y) max_y = val;
        h->SetBinContent(ibin,val);
    }
    h->SetMinimum(0.9*min_y);
    h->SetMaximum(1.1*max_y);

    if(!store(h))
    {
        log->warn("{0} - Failed to store baseline summary histogram (name = \"{1}\")",__VTFUNC__,h->GetName());
    }
    delete h;

    // graph it
    TGraphErrors* g = new TGraphErrors(m_channel_baseline_means.size());
    g->SetName("g_channel_baselines");
    g->SetTitle("g_channel_baselines");
    for(size_t i = 0; i < m_channel_baseline_means.size(); i++)
    {
        float val = m_channel_baseline_means.at(i);
        float err = m_channel_baseline_errors.at(i);
        g->SetPoint(i,i,val);
        g->SetPointError(i, 0, err);
    }
    g->SetMarkerStyle(20);
    g->SetMarkerColor(kBlack);
    g->SetLineColor(kBlack);
    if(!store(g))
    {
        log->warn("{0} - Failed to store baseline summary graph",__VTFUNC__);
    }
    delete g;
    

    return true;
}

bool VTSTestBaselines::finalize()
{
    for(size_t i = 0; i < m_channel_histos.size(); i++)
    {
        TH1* h = m_channel_histos.at(i);
        if(!h) { h = 0; continue; }
        if(!store(h))
        {
            log->warn("{0} - Failed to store xADC samples  histogram with name \"{1}\"",__VTFUNC__,h->GetName());
            continue;
        }
        delete h;
    }
    m_channel_histos.clear();

    return true;
}

json VTSTestBaselines::get_results()
{
    json jresults = {
        {"RESULT",VTSTestResultToStr(VTSTestResult::SUCCESS)}
    };
    return jresults;
}

void VTSTestBaselines::check_status()
{
    float frac = event_fraction_processed();
    //log->info("{0} - Check status: {1}",__VTFUNC__, frac);
    emit signal_status_update(frac);
}

void VTSTestBaselines::reset_vmm()
{
    comm()->configure_vmm(m_base_vmm_config, /*perform reset*/ true);
}

} // namespace vts
