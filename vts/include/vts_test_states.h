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
        NONE=0
        ,INITIALIZING
        ,INITIAL
        ,LOADING
        ,READY
        ,CONFIGURED
        ,RUNNING
        ,ANALYZING
        ,ANALYZINGTEST
        ,FINALIZING
        ,FINISHED
        ,TESTCOMPLETE
        ,TESTSTATEINVALID
    };
    std::string VTSTestStateToStr(const VTSTestState& s);
    VTSTestState StrToVTSTestState(const std::string& s);

} // namespace vts;

#endif
