#ifndef VTS_TEST_ChannelsAlive_H
#define VTS_TEST_ChannelsAlive_H

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
#include <chrono>

//ROOT
class TH1F;

namespace vts
{

class VTSTestChannelsAlive : public VTSTestImp
{
    public :
        explicit VTSTestChannelsAlive()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            m_total_events_processed.store(0);
            m_n_total_events.store(0);
        }
        ~VTSTestChannelsAlive()
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
            int cycle;
        };

        std::vector<TestStep> m_test_steps;

        int n_cycles;
        int n_cktp_per_cycle;
        int m_time_per_cycle; // milliseconds
        std::chrono::system_clock::time_point m_start_time;

        // put histograms/trees/any output here
        std::vector<TH1F*> h_vec_channels_cycle;
        TH1F* h_channel_occ_total;
        TH1F* h_channel_eff_total;

        // test results based on this
        std::vector<int> m_dead_channels;

}; // class VTSTestChannelsAlive

} // namespace vts

#endif
