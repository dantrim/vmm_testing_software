//vts
#include "vts_commands.h"

//std/stl
using namespace std;

namespace vts
{

string CMDFrontEndToStr(const CMDFrontEnd& cmd)
{
    string out = "CMDFrontEndInvalid";
    switch(cmd)
    {
        case CMDFrontEnd::POWERON : { out = "POWERON"; break; }
        case CMDFrontEnd::POWEROFF : { out = "POWEROFF"; break; }
        case CMDFrontEnd::PINGFPGA : { out = "PINGFPGA"; break; }
        case CMDFrontEnd::CONFIGUREFPGA : { out = "CONFIGUREFPGA"; break; }
        case CMDFrontEnd::CONFIGUREVMM : { out = "CONFIGUREVMM" ; break; }
        case CMDFrontEnd::ACQON : { out = "ACQON"; break; }
        case CMDFrontEnd::ACQOFF : { out = "ACQOFF"; break; }
        case CMDFrontEnd::RESETVMM : { out = "RESETVMM"; break; }
        case CMDFrontEnd::RESETFPGA : { out = "RESETFPGA"; break; }
        case CMDFrontEnd::CMDFrontEndInvalid : { out = "CMDFrontEndInvalid"; break; }
        case CMDFrontEnd::NCMDFrontEnd : { out = "NCMDFrontEnd"; break; }
    }
    return out;
}
CMDFrontEnd StrToCMDFrontEnd(const string& cmd)
{
    CMDFrontEnd out = CMDFrontEnd::CMDFrontEndInvalid;
    if(cmd == "POWERON") { out = CMDFrontEnd::POWERON; }
    else if(cmd == "POWEROFF") { out = CMDFrontEnd::POWEROFF; }
    else if(cmd == "PINGFPGA") { out = CMDFrontEnd::PINGFPGA; }
    else if(cmd == "CONFIGUREFPGA") { out = CMDFrontEnd::CONFIGUREFPGA; }
    else if(cmd == "CONFIGUREVMM") { out = CMDFrontEnd::CONFIGUREVMM; }
    else if(cmd == "ACQON") { out = CMDFrontEnd::ACQON; }
    else if(cmd == "ACQOFF") { out = CMDFrontEnd::ACQOFF; }
    else if(cmd == "RESETVMM") { out = CMDFrontEnd::RESETVMM; }
    else if(cmd == "RESETFPGA") { out = CMDFrontEnd::RESETFPGA;  }
    else { out = CMDFrontEnd::CMDFrontEndInvalid; }
    return out;
}



} // namespace vts
