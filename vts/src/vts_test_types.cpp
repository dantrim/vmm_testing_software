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
        case VTSTestType::Baselines : { out = "Baselines"; break; }
        case VTSTestType::ConfigurableVMM : { out = "ConfigurableVMM"; break; }
        case VTSTestType::BaselinesNeg : { out = "BaselinesNeg"; break; }
        case VTSTestType::BaselinesPos : { out = "BaselinesPos"; break; }
        case VTSTestType::ChannelsAliveNeg : { out = "ChannelsAliveNeg"; break; }
        case VTSTestType::VTSTESTTYPEINVALID : { out = "VTSTESTTYPEINVALID"; break; }
    }
    return out;
}

VTSTestType StrToVTSTestType(const string& t)
{
    VTSTestType out = VTSTestType::VTSTESTTYPEINVALID;
    if(t == "PassThrough") { out = VTSTestType::PassThrough; }
    else if(t == "Baselines") { out = VTSTestType::Baselines; }
    else if(t == "ConfigurableVMM") { out = VTSTestType::ConfigurableVMM; }
    else if(t == "BaselinesNeg") { out = VTSTestType::BaselinesNeg; }
    else if(t == "BaselinesPos") { out = VTSTestType::BaselinesPos; }
    else if(t == "ChannelsAliveNeg") { out = VTSTestType::ChannelsAliveNeg; }
    else { out = VTSTestType::VTSTESTTYPEINVALID; }
    return out;
}



} // namespace vts
