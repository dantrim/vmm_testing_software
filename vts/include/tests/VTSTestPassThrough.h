#ifndef VTS_TEST_PASS_THROUGH_H
#define VTS_TEST_PASS_THROUGH_H

//vts
#include "vts_test_imp.h"

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
        }
        ~VTSTestPassThrough()
        {
        }

        // METHODS TO IMPLEMENT
        bool initialize(const json& config);
        bool load();
        bool configure();
        bool run();
        bool analyze();
        bool analyze_test();
        bool finalize();
        json get_results();

    private :

}; // class VTSTestPassThrough

} // namespace vts

#endif
