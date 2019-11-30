#ifndef VTS_TEST_ChannelsAliveNeg_H
#define VTS_TEST_ChannelsAliveNeg_H

//vts
#include "vts_test_imp.h"
namespace vts {
    namespace daq {
        class DataFragment;
    }
}

//logging
#include "spdlog/spdlog.h"

//std/stl
#include <vector>

//ROOT
class TH1F;

namespace vts
{

class VTSTestChannelsAliveNeg : public VTSTestImp
{
    public :
        explicit VTSTestChannelsAliveNeg()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            set_n_events_per_step(1000);
            m_total_events_processed.store(0);
            m_n_total_events.store(0);
        }
        ~VTSTestChannelsAliveNeg()
        {
        }

        // METHODS TO IMPLEMENT
        bool initialize(const json& config);
        bool load();
        bool configure();
        bool run();
        bool process_event(vts::daq::DataFragment* fragment);
        bool analyze();
        bool analyze_test();
        bool finalize();
        json get_results();

    private :

        struct TestStep
        {
            int channel;
            int cycle;
        };

        std::vector<TestStep> m_test_steps;

        // put histograms/trees/any output here
        TH1F* h_channel_occupancy;
        TH1F* h_channel_eff;
        std::vector<TH1F*> m_cycle_occ_histos;
        std::vector<TH1F*> m_cycle_eff_histos;
        std::vector<int> m_dead_channels;
        int n_times;
        TH1F* h_over_count;

}; // class VTSTestChannelsAliveNeg

} // namespace vts

#endif
