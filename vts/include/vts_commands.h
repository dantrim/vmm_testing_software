#ifndef VTS_COMMANDS_H
#define VTS_COMMANDS_H

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
        ,ACQON
        ,ACQOFF
        ,CMDFrontEndInvalid
        ,NCMDFrontEnd
    };
    std::string CMDFrontEndToStr(const CMDFrontEnd& cmd);
    CMDFrontEnd StrToCMDFrontEnd(const std::string& cmd);

} // namespace vts

#endif
