#ifndef VTS_RESULT_H
#define VTS_RESULT_H

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
