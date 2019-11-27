#ifndef VTS_TEST_PASSTHROUGH_H
#define VTS_TEST_PASSTHROUGH_H

//std/stl

//vts
#include "vts_test_imp.h"
namespace vts {
    namespace daq {
        class DataFragment;
    }
}

//ROOT
class TH2F;

//logging
#include "spdlog/spdlog.h"

namespace vts
{

class VTSTestPassThrough : public VTSTestImp
{
    public :
        explicit VTSTestPassThrough()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            set_n_events_per_step(1000);
            m_total_events_processed.store(0);
            m_n_total_events.store(0);
        }
        ~VTSTestPassThrough()
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
            std::string pulse_width;
        };
        std::vector<TestStep> m_test_steps;
        TH2F* h2_channel_hit_vs_pdo;
        TH2F* h2_channel_hit_vs_tdo;

}; // class VTSTestPassThrough

} // namespace vts

#endif
