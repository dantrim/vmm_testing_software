// vts
#include "helpers.h"
#include "communicator_frontend.h"

//std/stl
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

// logging
#include "spdlog/spdlog.h"

// Qt
//#include <QProcess>

namespace vts
{

CommunicatorFrontEnd::CommunicatorFrontEnd(json config)
{
    log = spdlog::get("vts_logger");
    log->info("{0} - {1}",__VTFUNC__,"CommunicatorFrontEnd starts");
    load_config(config);
}

CommunicatorFrontEnd::~CommunicatorFrontEnd()
{
    log->info("{0} - {1}",__VTFUNC__,"CommunicatorFrontEnd ends");
}

void CommunicatorFrontEnd::load_config(json config)
{
    try
    {
        m_board_ip = config.at("board_ip");
    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "Problem loading front-end configuration data: " << e.what();
        throw std::runtime_error(err.str());
    }
}

bool CommunicatorFrontEnd::power_board_toggle(bool turn_on)
{
    if(turn_on)
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning frontend ON");
    }
    else
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning frontend OFF");
    }
    return true;
}

bool CommunicatorFrontEnd::ping_fpga()
{
    stringstream msg;
    msg << "Pinging FPGA at IP: " << m_board_ip;
    log->info("{0} - {1}",__VTFUNC__,msg.str());

    msg.str("");
    msg << "ping " << m_board_ip << " -c 1 -W 1 > /dev/null";
    int status_code = system(msg.str().c_str());
    if(status_code != 0)
        return false;
    return true;
}

bool CommunicatorFrontEnd::configure_fpga()
{
    log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Configuring FPGA");
    return true;
}

bool CommunicatorFrontEnd::acq_toggle(bool turn_on)
{
    if(turn_on)
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning ACQ ON");
    }
    else
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning ACQ OFF");
    }
    return true;
}

} // namespace vts
