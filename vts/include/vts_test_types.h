#ifndef VTS_TEST_TYPES_H
#define VTS_TEST_TYPES_H

//stdstl
#include <string>

namespace vts
{
    ///////////////////////////////////////////////////////////
    // VTS TYPE
    ///////////////////////////////////////////////////////////
    enum VTSTestType
    {
        PassThrough=0
        ,Baselines
        ,ConfigurableVMM
        ,BaselinesNeg
        ,BaselinesPos
        ,ChannelsAliveNeg
        ,ChannelsAlivePos
        ,VTSTESTTYPEINVALID
    };
    std::string VTSTestTypeToStr(const VTSTestType& t);    
    VTSTestType StrToVTSTestType(const std::string& t);

} // namespace vts

#endif
