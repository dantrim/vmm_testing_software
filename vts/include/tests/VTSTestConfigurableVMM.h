#ifndef VTS_TEST_ConfigurableVMM_H
#define VTS_TEST_ConfigurableVMM_H

//vts
#include "vts_test_imp.h"
namespace vts {
    namespace daq {
        class DataFragment;
    }
}

//std/stl
#include <vector>
#include <map>

//logging
#include "spdlog/spdlog.h"

//ROOT
class TH1F;

namespace vts
{

class VTSTestConfigurableVMM : public VTSTestImp
{
    public :
        explicit VTSTestConfigurableVMM()
        {
            log = spdlog::get("vts_logger");
            set_current_state(-1);
            set_n_states(1);
            set_n_events_per_step(50);
            m_total_events_processed.store(0);
            m_n_total_events.store(0);
        }
        ~VTSTestConfigurableVMM()
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
            bool is_configured;
            std::string channel;
        };
        std::vector<TestStep> m_test_steps;

        // put histograms/trees/any output here
        std::vector<TH1F*> m_histos_baselines_not_configured;
        std::vector<TH1F*> m_histos_baselines_configured;
        std::map<int, int> map_chan_to_idx;
        TH1F* m_histo_noise_ratio;
        bool m_vmm_configurable;
        

}; // class VTSTestConfigurableVMM

} // namespace vts

#endif
