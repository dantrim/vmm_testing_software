//vts
#include "frontend_address.h"

//std/stl
using namespace std;

namespace vts
{

string FPGARegisterToStr(const FPGARegister& reg)
{
    string out = "FPGAREGISTERINVALID";
    switch(reg)
    {
        case FPGARegister::TRIGGERMODE : { out = "TRIGGERMODE"; break; }
        case FPGARegister::DAQMODE : { out = "DAQMODE"; break; }
        case FPGARegister::LATENCY : { out = "LATENCY"; break; }
        case FPGARegister::READOUTMODE : { out = "READOUTMODE"; break; }
        case FPGARegister::FPGARESET : { out = "FPGARESET"; break; }
        case FPGARegister::CKTKMAXNUMBER : { out = "CKTKMAXNUMBER"; break; }
        case FPGARegister::CKBCFREQUENCY : { out = "CKBCFREQUENCY"; break; }
        case FPGARegister::CKTPMAXNUMBER : { out = "CKTPMAXNUMBER"; break; }
        case FPGARegister::CKTPSKEW : { out = "CKTPSKEW"; break; }
        case FPGARegister::CKTPPERIOD : { out = "CKTPPERIOD"; break; }
        case FPGARegister::CKTPWIDTH : { out = "CKTPWIDTH"; break; }
        case FPGARegister::CKBCMAXNUMBER : { out = "CKBCMAXNUMBER"; break; }
        case FPGARegister::TRIGGERDEADTIME : { out = "TRIGGERDEADTIME"; break; }
        case FPGARegister::ARTTIMEOUT : { out = "ARTTIMEOUT"; break; }
        case FPGARegister::LATENCYEXTRACKBC : { out = "LATENCYEXTRACKBC"; break; }
        case FPGARegister::ANALOGMODE : { out = "ANALOGMODE"; break; }
        case FPGARegister::FPGAREGISTERINVALID : { out = "FPGAREGISTERINVALID"; break; }
    }
    return out;
}

FPGARegister StrToFPGARegister(const std::string& reg)
{
    FPGARegister out = FPGARegister::FPGAREGISTERINVALID;
    if(reg == "TRIGGERMODE")                { out = FPGARegister::TRIGGERMODE; }
    else if(reg == "DAQMODE")               { out = FPGARegister::DAQMODE; }
    else if(reg == "LATENCY")               { out = FPGARegister::LATENCY; }
    else if(reg == "READOUTMODE")           { out = FPGARegister::READOUTMODE; }
    else if(reg == "FPGARESET")             { out = FPGARegister::FPGARESET; }
    else if(reg == "CKTKMAXNUMBER")         { out = FPGARegister::CKTKMAXNUMBER; }
    else if(reg == "CKBCFREQUENCY")         { out = FPGARegister::CKBCFREQUENCY; }
    else if(reg == "CKTPMAXNUMBER")         { out = FPGARegister::CKTPMAXNUMBER; }
    else if(reg == "CKTPSKEW")              { out = FPGARegister::CKTPSKEW; }
    else if(reg == "CKTPPERIOD")            { out = FPGARegister::CKTPPERIOD; }
    else if(reg == "CKTPWIDTH")             { out = FPGARegister::CKTPWIDTH; }
    else if(reg == "CKBCMAXNUMBER")         { out = FPGARegister::CKBCMAXNUMBER; }
    else if(reg == "TRIGGERDEADTIME")       { out = FPGARegister::TRIGGERDEADTIME; }
    else if(reg == "ARTTIMEOUT")            { out = FPGARegister::ARTTIMEOUT; }
    else if(reg == "LATENCYEXTRACKBC")      { out = FPGARegister::LATENCYEXTRACKBC; }
    else if(reg == "ANALOGMODE")            { out = FPGARegister::ANALOGMODE; }
    else                                    { out = FPGARegister::FPGAREGISTERINVALID; }
    return out;
}

string XADCRegisterToStr(const XADCRegister& reg)
{
    string out = "XADCREGISTERINVALID";
    switch(reg)
    {
        case XADCVMMID : { out = "XADCVMMID"; break; }
        case XADCSAMPLESIZE : { out = "XADCSAMPLESIZE"; break; }
        case XADCDELAY : { out = "XADCDELAY"; break; }
        case XADCCKTPENABLE : { out = "XADCCKTPENABLE"; break; }
        case XADCREGISTERINVALID : { out = "XADCREGISTERINVALID"; break; }
    }
    return out;
}

XADCRegister StrToXADCRegister(const string& reg)
{
    XADCRegister out = XADCRegister::XADCREGISTERINVALID;
    if(reg == "XADCVMMID")               { out = XADCRegister::XADCVMMID; } 
    else if(reg == "XADCSAMPLESIZE")     { out = XADCRegister::XADCSAMPLESIZE; }
    else if(reg == "XADCDELAY")          { out = XADCRegister::XADCDELAY; }
    else if(reg == "XADCCKTPENABLE")     { out = XADCRegister::XADCCKTPENABLE; }
    else                                 { out = XADCRegister::XADCREGISTERINVALID; }
    return out;
}

} // namespace vts
