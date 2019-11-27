#ifndef VTS_TEST_SKELETONTEST_H
#define VTS_TEST_SKELETONTEST_H

//vts
#include "vts_test_imp.h"
namespace vts {
    namespace daq {
        class DataFragment;
    }
}

//logging
#include "spdlog/spdlog.h"

namespace vts
{

class VTSTestSKELETONTEST : public VTSTestImp
{
    public :
        explicit VTSTestSKELETONTEST()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            set_n_events_per_step(1000);
            m_total_events_processed.store(0);
            m_n_total_events.store(0);
        }
        ~VTSTestSKELETONTEST()
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
            std::string channel;
            std::string pulse_width;
            std::string dac_pulser;
        };
        std::vector<TestStep> m_test_steps;

        // put histograms/trees/any output here

}; // class VTSTestSKELETONTEST

} // namespace vts

#endif
