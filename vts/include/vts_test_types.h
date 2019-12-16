#ifndef VTS_TEST_TYPES_H
#define VTS_TEST_TYPES_H

/////////////////////////////////////////////////////
//
// vts_test_types.h
//
// the types of tests recognized by VTS
//
// daniel joseph antrim
// october 2019
// daniel.joseph.antrim@cern.ch
//
/////////////////////////////////////////////////////

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
