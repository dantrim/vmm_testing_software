#ifndef VTS_TEST_STATES_H
#define VTS_TEST_STATES_H

/////////////////////////////////////////////////////
//
// vts_test_states.h
//
// states that any given test can be in, should follow
// a finite state machine
//
// daniel joseph antrim
// october 2019
// daniel.joseph.antrim@cern.ch
//
/////////////////////////////////////////////////////

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
