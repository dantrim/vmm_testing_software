#ifndef VTS_TEST_PASS_THROUGH_H
#define VTS_TEST_PASS_THROUGH_H

//std/stl

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

class VTSTestPassThrough : public VTSTestImp
{
    public :
        explicit VTSTestPassThrough()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            set_n_events_per_step(1000);
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

        void reset_vmm();

        json m_test_data;
        json m_base_fpga_config;
        json m_base_vmm_config;
        struct TestStep
        {
            //std::string global_threshold_dac;
            //std::string global_pulser_dac;
            std::string pulse_width;
        };
        std::vector<TestStep> m_test_steps;

}; // class VTSTestPassThrough

} // namespace vts

#endif
