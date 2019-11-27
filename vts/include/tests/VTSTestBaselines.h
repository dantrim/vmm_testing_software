#ifndef VTS_TEST_BASELINES_H
#define VTS_TEST_BASELINES_H

//std/stl

//vts
#include "vts_test_imp.h"
namespace vts {
    namespace daq {
        class DataFragment;
    }
}

//ROOT
class TTree;
class TH1F;

//logging
#include "spdlog/spdlog.h"

namespace vts
{

class VTSTestBaselines : public VTSTestImp
{
    public :
        explicit VTSTestBaselines()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            set_n_events_per_step(50);
            m_total_events_processed.store(0);
            m_n_total_events.store(0);
        }
        ~VTSTestBaselines()
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

        json m_test_data;
        json m_base_fpga_config;
        json m_base_vmm_config;
        struct TestStep
        {
            std::string channel;
        };
        std::vector<TestStep> m_test_steps;

        // test data
        std::vector<TH1F*> m_channel_histos;
        std::vector<float> m_channel_baseline_means;
        std::vector<float> m_channel_baseline_errors;

}; // class VTSTestBaselines

} // namespace vts

#endif
