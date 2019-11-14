//vts
#include "tests/VTSTestPassThrough.h"
#include "helpers.h"

//std/stl
#include <sstream>
using namespace std;

//logging
#include "spdlog/spdlog.h"

namespace vts
{

VTSTestPassThrough::VTSTestPassThrough()
{
    log = spdlog::get("vts_logger");
}
VTSTestPassThrough::~VTSTestPassThrough()
{
}
bool VTSTestPassThrough::initialize(const json& config)
{
    stringstream msg;
    msg << "Initializing with config: " << config.dump();
    log->info("{0} - {1}",__VTFUNC__,msg.str());
    return true;
}


} // namespace vts
