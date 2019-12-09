#ifndef VTS_TEST_ChannelsAliveSingle_H
#define VTS_TEST_ChannelsAliveSingle_H

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
class TH2F;

namespace vts
{

class VTSTestChannelsAliveSingle : public VTSTestImp
{
    public :
        explicit VTSTestChannelsAliveSingle()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            set_n_events_per_step(50);
            m_total_events_processed.store(0);
            m_n_total_events.store(0);
        }
        ~VTSTestChannelsAliveSingle()
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
        };
        std::vector<TestStep> m_test_steps;

        // put histograms/trees/any output here
        TH1F* h_channel;
        TH2F* h2_pdo;
        int n_cktp_per_cycle;
        int n_cycles;
        int m_time_per_cycle;

}; // class VTSTestChannelsAliveSingle

} // namespace vts

#endif
