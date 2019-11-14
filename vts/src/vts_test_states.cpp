// vts
#include "vts_test_states.h"

//std/stl
#include <string>
using namespace std;

namespace vts
{

string VTSTestStateToStr(const VTSTestState& s)
{
    string out = "TESTSTATEINVALID";
    switch(s)
    {
        case VTSTestState::INITIAL : { out = "INITIAL"; break; }
        case VTSTestState::LOADING : { out = "LOADING"; break; }
        case VTSTestState::READY : { out = "READY"; break; }
        case VTSTestState::CONFIGURED : { out = "CONFIGURED"; break; }
        case VTSTestState::RUNNING : { out = "RUNNING"; break; }
        case VTSTestState::ANALYZING : { out = "ANALYZING"; break; }
        case VTSTestState::ANALYZINGTEST : { out = "ANALYZINGTEST"; break; } 
        case VTSTestState::FINISHED : { out = "FINISHED"; break; }
        case VTSTestState::TESTSTATEINVALID : { out = "TESTSTATEINVALID"; break; }
    }
    return out;
}

VTSTestState StrToVTSTestState(const std::string& s)
{
    VTSTestState out = VTSTestState::TESTSTATEINVALID;
    if(s == "INITIAL")                      { out = VTSTestState::INITIAL; }
    else if(s == "LOADING")                 { out = VTSTestState::LOADING; }
    else if(s == "READY")                   { out = VTSTestState::READY; }
    else if(s == "CONFIGURED")              { out = VTSTestState::CONFIGURED; }
    else if(s == "RUNNING")                 { out = VTSTestState::RUNNING; }
    else if(s == "ANALYZING")               { out = VTSTestState::ANALYZING; }
    else if(s == "ANALYZINGTEST")           { out = VTSTestState::ANALYZINGTEST; }
    else if(s == "FINISHED")                { out = VTSTestState::FINISHED; }
    else                                    { out = VTSTestState::TESTSTATEINVALID; }
    return out;
}

} // namespace vts
