#ifndef VTS_DAQ_DEFS_H
#define VTS_DAQ_DEFS_H

/////////////////////////////////////////////////////
//
// daq_defs.h
//
// typedefs to define the format in which the buffer
// queues data
//
// daniel joseph antrim
// october 2019
// daniel.joseph.antrim@cern.ch
//
/////////////////////////////////////////////////////


//moodycamel
#include "concurrentqueue/concurrentqueue.h"
using moodycamel::ConcurrentQueue;

//boost
#include <boost/array.hpp>

//vts
#include "daq_data_fragment.h"

#define MAX_UDP_LEN 2500
typedef boost::array<uint32_t, MAX_UDP_LEN> data_array_t;

namespace vts
{
namespace daq
{
    typedef ConcurrentQueue<vts::daq::DataFragment*> DataQueue;
} // namespace daq
} // namespace vts

#endif
