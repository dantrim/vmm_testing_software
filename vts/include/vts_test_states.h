#ifndef VTS_TEST_STATES_H
#define VTS_TEST_STATES_H

//std/stl
#include <string>

namespace vts
{
    ///////////////////////////////////////////////////////////
    // VTS STATES
    ///////////////////////////////////////////////////////////
    enum VTSTestState
    {
        INITIAL=0
        ,LOADING
        ,READY
        ,CONFIGURED
        ,RUNNING
        ,ANALYZING
        ,ANALYZINGTEST
        ,FINISHED
        ,TESTSTATEINVALID
    };
    std::string VTSTestStateToStr(const VTSTestState& s);
    VTSTestState StrToVTSTestState(const std::string& s);

} // namespace vts;

#endif
