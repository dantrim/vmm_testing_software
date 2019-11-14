//vts
#include "tests/VTSTestPassThrough.h"
#include "helpers.h"

//std/stl
#include <sstream>
#include <chrono>
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
    std::this_thread::sleep_for(std::chrono::seconds(5));
    msg.str("");
    msg << "INITIALIZED!";
    log->info("{0} - {1}",__VTFUNC__,msg.str());
    return true;
}

bool VTSTestPassThrough::start()
{
    stringstream msg;
    msg << "Starting test";
    log->info("{0} - {1}",__VTFUNC__,msg.str());
    std::this_thread::sleep_for(std::chrono::seconds(5));
    msg.str("");
    msg << "STARTED!";
    log->info("{0} - {1}",__VTFUNC__,msg.str());
    log->info("{0} - {1}",__VTFUNC__,"Initializing Outputs etc");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    log->info("{0} - {1}",__VTFUNC__,"Establishing DAQ");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    log->info("{0} - {1}",__VTFUNC__,"Collecting data!");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    emit finished();
    
    return true;
}


} // namespace vts
