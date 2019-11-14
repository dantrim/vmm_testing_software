//vts/
#include "vts_result.h"

//std/st/
using namespace std;

namespace vts
{

string VTSTestResultToStr(const VTSTestResult& rst)
{
    string out = "TESTRESULTINVALID";
    switch(rst)
    {
        case VTSTestResult::SUCCESS : { out = "SUCCESS"; break; }
        case VTSTestResult::PASS : { out = "PASS"; break; }
        case VTSTestResult::INCOMPLETE : { out = "INCOMPLETE"; break; }
        case VTSTestResult::FAIL : { out = "FAIL"; break; }
        case VTSTestResult::TESTRESULTINVALID : { out = "TESTRESULTINVALID"; break; }
    }
    return out;
}

VTSTestResult StrToVTSTestResult(const string& rst)
{
    VTSTestResult out = VTSTestResult::TESTRESULTINVALID;
    if(rst == "SUCCESS") { out = VTSTestResult::SUCCESS; }
    else if(rst == "PASS") { out = VTSTestResult::PASS; }
    else if(rst == "INCOMPLETE") { out = VTSTestResult::INCOMPLETE; }
    else if(rst == "FAIL") { out = VTSTestResult::FAIL; }
    else { out = VTSTestResult::TESTRESULTINVALID; }
    return out;
}

} // namespace vts
