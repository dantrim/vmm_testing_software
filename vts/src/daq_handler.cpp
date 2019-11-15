//vts
#include "helpers.h"
#include "daq_handler.h"

//boost
#include <boost/bind.hpp>

//std/stl
#include <string>
#include <sstream>
#include <iostream>
using namespace std;

//logging
#include "spdlog/spdlog.h"


namespace vts
{
namespace daq
{

DaqHandler::DaqHandler(QObject* parent) :
    QObject(parent),
    m_is_running(false)
{
    log = spdlog::get("vts_logger");
    m_io_service = std::make_shared<boost::asio::io_service>();
}

void DaqHandler::load_connections(const json& frontend_config, const json& daq_config)
{
    stringstream msg;
    msg << "Frontend config: " << frontend_config.dump();
    log->info("{0} - {1}",__VTFUNC__,msg.str());
    msg.str("");
    msg << "DAQ config: " << daq_config.dump();
    log->info("{0} - {1}",__VTFUNC__, msg.str());

    // frontend config
    string board_ip = frontend_config.at("board_ip").get<std::string>();
    uint32_t ip = string_ip_to_int(board_ip);
    m_listen_ip = ip;

    // daq config
    string recv_port = daq_config.at("recv_port");
    uint32_t port = std::stoi(recv_port);
    m_listen_port = port;

    return;
}

void DaqHandler::start_listening()
{
    log->info("{0}",__VTFUNC__);

    m_listen_flag.store(true);
    m_build_flag.store(true);

    // queue to read in the incoming data
    vts::daq::DataQueue* queue = new vts::daq::DataQueue(1023);

    // configure the listener
    vts::daq::DataListener* listener = new vts::daq::DataListener(m_listen_ip, m_listen_port,
            m_io_service,
            queue,
            std::ref(m_listen_flag)
        );

    if(!listener->connect())
    {
        stringstream err;
        err << "DataListener could not connect to port " << m_listen_port;
        throw std::runtime_error(err.str());
    }

    // queue up the data listener with the io service
    m_io_service->post(boost::bind(&vts::daq::DataListener::listen, listener));
    m_listen_queues.push_back(queue);
    m_listeners.push_back(listener);

    // start the io-service
    size_t n_listeners = m_listeners.size();
    for(size_t i = 0; i < n_listeners; i++)
    {
        m_listeners.at(i)->start();
    } // i
    m_is_running = true;
}

void DaqHandler::stop_listening()
{
    log->info("{0}",__VTFUNC__);
    

    size_t n_listeners = m_listeners.size();
    m_listen_flag.store(false);
    m_build_flag.store(false);

    // give the listeners and builders a bit of time to react to their flags changing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if(m_io_service)
    {
        log->debug("{0} - Stopping IO service", __VTFUNC__);
        m_io_service->stop();
    }

    for(size_t i = 0; i < n_listeners; i++)
    {
        m_listeners.at(i)->stop();
        //m_listeners.at(i)->shutdown();
    }

//    for(size_t i = 0; i < n_listeners; i++)
//    {
//        m_builders.at(i)->stop();
//    }

    for(size_t i = 0; i < m_listen_queues.size(); i++)
    {
        //delete m_builders.at(i);
        delete m_listen_queues.at(i);
        delete m_listeners.at(i);
    }

    m_listen_queues.clear();
    m_listeners.clear();
    //m_builders.clear();
    m_is_running = false;
}


} // namespace daq
} // namespace vts
