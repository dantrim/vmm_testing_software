#ifndef VTS_FRONTEND_ADDRESSES_H
#define VTS_FRONTEND_ADDRESSES_H

/////////////////////////////////////////////////////
//
// frontend_address.h
//
// addresses of anything to which we send commands to
// on the frontend
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

    //
    // reference:
    // https://docs.google.com/spreadsheets/d/1qIYg77oNUx_fnSCMp6WCNwcYpQFnnKblu6DFhKbPMT4/edit#gid=0
    //

    //////////////////////////////////////////////////
    // FPGA
    //////////////////////////////////////////////////
    enum FPGARegister
    {
        TRIGGERMODE = 0xAB // default value : 0x07
        ,DAQMODE = 0x0F // default value : 0x00
        ,LATENCY = 0x05 // default value : 0x0000
        ,READOUTMODE = 0xCD // default value : 0x00
        ,FPGARESET = 0xAF // default value : 0x00
        // clocks below
        ,CKTKMAXNUMBER = 0xC1 // default value : 0x07
        ,CKBCFREQUENCY = 0xC2 // default value : 0x28
        ,CKTPMAXNUMBER = 0xC3 // default value : 0xFFFF
        ,CKTPSKEW = 0xC4 // default value : 0x00
        ,CKTPPERIOD = 0xC5 // default value : 0x7530
        ,CKTPWIDTH = 0xC6 // default value : 0x04
        // trigger below
        ,CKBCMAXNUMBER = 0xC7 // default value : 0x06
        ,TRIGGERDEADTIME = 0xC8 // default value : 0x2000
        ,ARTTIMEOUT = 0xC9 // default value : 0x18
        ,LATENCYEXTRACKBC = 0xCA // default value : 0x0058
        ,ANALOGMODE = 0xF5 // default value : 0x00
        ,FPGAREGISTERINVALID = 0x00
    };
    std::string FPGARegisterToStr(const FPGARegister& reg);
    FPGARegister StrToFPGARegister(const std::string& reg);

    //////////////////////////////////////////////////
    // xADC
    //////////////////////////////////////////////////
    enum XADCRegister
    {
        XADCVMMID = 0xA1 // default value : 0x0000
        ,XADCSAMPLESIZE = 0xA2 // default value : 0x3FF
        ,XADCDELAY = 0xA3 // default value : 0x1FFFF
        ,XADCCKTPENABLE = 0xA4 // default value : 0x0
        ,XADCREGISTERINVALID = 0x00
    };
    std::string XADCRegisterToSTr(const XADCRegister& reg);
    XADCRegister StrToXADCRegister(const std::string& reg);

    

} // namespace vts

#endif
