#ifndef VTS_COMMANDS_H
#define VTS_COMMANDS_H

/////////////////////////////////////////////////////
//
// vts_commands.h
//
// recognized commands that the VTS server should
// respond to
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
    // FRONTEND COMMANDS
    ///////////////////////////////////////////////////////////
    enum CMDFrontEnd
    {
        POWERON=0
        ,POWEROFF
        ,PINGFPGA
        ,CONFIGUREFPGA
        ,CONFIGUREVMM
        ,ACQON
        ,ACQOFF
        ,RESETVMM
        ,RESETFPGA
        ,CMDFrontEndInvalid
    };
    std::string CMDFrontEndToStr(const CMDFrontEnd& cmd);
    CMDFrontEnd StrToCMDFrontEnd(const std::string& cmd);

    ///////////////////////////////////////////////////////////
    // TESTS
    ///////////////////////////////////////////////////////////
    enum CMDVMMTest
    {
        LOAD=0
        ,START
        ,STOP
        ,CMDVMMTESTINVALID
    };
    std::string CMDVMMTestToStr(const CMDVMMTest& cmd);
    CMDVMMTest StrToCMDVMMTest(const std::string& cmd);

} // namespace vts

#endif
