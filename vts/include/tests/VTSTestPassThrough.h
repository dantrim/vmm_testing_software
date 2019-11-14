#ifndef VTS_TEST_PASS_THROUGH_H
#define VTS_TEST_PASS_THROUGH_H

#include "vts_test_imp.h"
////json
//#include "nlohmann/json.hpp"
//using json = nlohmann::json;

namespace vts
{

class VTSTestPassThrough : public VTSTestImp
{
    public :
        explicit VTSTestPassThrough();
        ~VTSTestPassThrough();
        bool initialize(const json& config);

    private :

}; // class VTSTestPassThrough

} // namespace vts

#endif
