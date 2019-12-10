#ifndef VTS_TEST_Baselines_H
#define VTS_TEST_Baselines_H

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
#include <map>

//ROOT
class TH1F;

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

        struct TestStep
        {
            std::string channel;
        };
        std::vector<TestStep> m_test_steps;

        bool need_to_redo_last_step();
        void redo_last_step();

        // put histograms/trees/any output here
        const float LO_BASELINE_THRESHOLD = 150.0;
        const float HI_BASELINE_THRESHOLD = 210.0;
        std::vector<TH1F*> m_histos_baselines;
        std::vector<float> m_channel_baseline_means;
        std::vector<float> m_channel_noise;
        std::vector<int> m_bad_baselines;
        TH1F* h_baseline_summary;
        TH1F* h_noise_summary;
        int n_bad_channels;

        const int N_RETRY_MAX = 3;
        std::map<int, int> m_retry_map;

}; // class VTSTestBaselines

} // namespace vts

#endif
