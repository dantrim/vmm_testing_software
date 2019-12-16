#ifndef VTS_RESULT_H
#define VTS_RESULT_H

/////////////////////////////////////////////////////
//
// vts_result.h
//
// definitions of the test results
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
    // TEST RESULT
    ///////////////////////////////////////////////////////////
    enum VTSTestResult
    {
        SUCCESS=0
        ,PASS
        ,INCOMPLETE
        ,FAIL
        ,TESTRESULTINVALID
    };
    std::string VTSTestResultToStr(const VTSTestResult& rst);
    VTSTestResult StrToVTSTestResult(const std::string& rst);

} // namespace vts

#endif
