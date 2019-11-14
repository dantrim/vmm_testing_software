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


bool VTSTestPassThrough::initialize(const json& config)
{
    stringstream msg;
    msg << "Initializing with config: " << config.dump();
    log->info("{0} - {1}",__VTFUNC__,msg.str());
    std::this_thread::sleep_for(std::chrono::seconds(5));
    msg.str("");
    msg << "INITIALIZED!";
    log->info("{0} - {1}",__VTFUNC__,msg.str());

    set_current_state(0);
    set_n_states(2);
    return true;
}

bool VTSTestPassThrough::load()
{
    set_current_state( get_current_state() + 1);

    log->info("{0}",__VTFUNC__);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool VTSTestPassThrough::configure()
{
    log->info("{0}",__VTFUNC__);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool VTSTestPassThrough::run()
{
    log->info("{0}",__VTFUNC__);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool VTSTestPassThrough::analyze()
{
    log->info("{0}",__VTFUNC__);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool VTSTestPassThrough::analyze_test()
{
    log->info("{0}",__VTFUNC__);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool VTSTestPassThrough::finalize()
{
    log->info("{0}",__VTFUNC__);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

json VTSTestPassThrough::get_results()
{
    json jresults = {
        {"RESULT",VTSTestResultToStr(VTSTestResult::SUCCESS)}
    };
    return jresults;
}

//bool VTSTestPassThrough::start()
//{
//    stringstream msg;
//    msg << "Starting test";
//    log->info("{0} - {1}",__VTFUNC__,msg.str());
//    std::this_thread::sleep_for(std::chrono::seconds(5));
//    msg.str("");
//    msg << "STARTED!";
//    log->info("{0} - {1}",__VTFUNC__,msg.str());
//    log->info("{0} - {1}",__VTFUNC__,"Initializing Outputs etc");
//    std::this_thread::sleep_for(std::chrono::seconds(2));
//    log->info("{0} - {1}",__VTFUNC__,"Establishing DAQ");
//    std::this_thread::sleep_for(std::chrono::seconds(2));
//    log->info("{0} - {1}",__VTFUNC__,"Collecting data!");
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//    
//    return true;
//}


} // namespace vts
