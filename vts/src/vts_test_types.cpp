//vts
#include "vts_test_types.h"

//std/stl
#include <string>
using namespace std;

namespace vts
{

string VTSTestTypeToStr(const VTSTestType& t)
{
    string out = "VTSTESTTYPEINVALID";
    switch(t)
    {
        case VTSTestType::PassThrough : { out = "PassThrough"; break; }
        case VTSTestType::VTSTESTTYPEINVALID : { out = "VTSTESTTYPEINVALID"; break; }
    }
    return out;
}

VTSTestType StrToVTSTestType(const string& t)
{
    VTSTestType out = VTSTestType::VTSTESTTYPEINVALID;
    if(t == "PassThrough") { out = VTSTestType::PassThrough; }
    else { out = VTSTestType::VTSTESTTYPEINVALID; }
    return out;
}



} // namespace vts
