//vts
#include "helpers.h"
#include "daq_listener.h"

//std/stl
#include <iostream>
#include <sstream>
using namespace std;

//boost
#include <boost/bind.hpp>

//logging
#include "spdlog/spdlog.h"

namespace vts
{
namespace daq
{

DataListener::DataListener(uint32_t listen_ip, uint32_t listen_port,
            std::shared_ptr<boost::asio::io_service> io_service,
            DataQueue* input_queue,
            std::atomic<bool>& listen_flag) :
    m_active(false),
    m_listen_ip(listen_ip),
    m_listen_port(listen_port)
{
    log = spdlog::get("vts_logger");
    m_io_service = io_service;
    m_in_queue = input_queue;
    m_listen_flag = & listen_flag;
}

bool DataListener::connect()
{
    if(!m_io_service)
    {
        log->error("{0} - Listener cannot connect, io_service is null", __VTFUNC__);
        return false;
    }

    try
    {
        if(m_socket && m_socket->is_open())
        {
            m_socket->close();
            boost::system::error_code ec;
            m_socket->shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
        }
        m_socket = std::make_shared<boost::asio::ip::udp::socket>(*m_io_service,
            boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), m_listen_port));
        return true;
    } // try
    catch(std::exception& e)
    {
        log->error("{0} - Failed to resolve IP endpoint for listener port {1}, exception: {2}",
            __VTFUNC__, m_listen_port, e.what());
        return false;
    } // catch

}

void DataListener::start()
{
    m_thread = std::thread( [this] ()
            {
                m_io_service->run();
            }
        );
    m_active = true;
}

bool DataListener::continue_listening()
{
    return m_listen_flag->load(std::memory_order_acquire);
}

void DataListener::shutdown()
{
    log->debug("{0} - Shutting down listener of port {1}", __VTFUNC__, m_listen_port);
    if(m_socket && m_socket->is_open())
    {
        m_socket->close();
        boost::system::error_code ec;
        m_socket->shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
    }
    m_socket.reset();
}

void DataListener::stop()
{
    if(!m_active) return;
    if(m_thread.joinable())
    {
        m_thread.join();
    }
    shutdown();
    m_active = false;
}

void DataListener::listen()
{
    if(continue_listening())
    {
        m_socket->async_receive(boost::asio::buffer(m_in_buffer),
            boost::bind(&DataListener::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
}

void DataListener::handle_receive(const boost::system::error_code& error, size_t n_bytes)
{
    if(!continue_listening()) return;
    if(n_bytes == 0 && m_in_buffer.size() == 0)
    //if(n_bytes == 0 && m_data_in.size() == 0)
    {
        listen();
    }

    size_t n32 = n_bytes / 4;
    m_data_in.clear();
    for(size_t i = 0; i < n32; i++) m_data_in.push_back(m_in_buffer.at(i));


    if(n32 == 0 && continue_listening())
    {
        listen();
    }

    DataFragment* fragment = new DataFragment();
    fragment->set_port_recv(m_listen_port);
    if(m_data_in.size()>0)
    {
        VMMHeader header = { (uint32_t)m_data_in.at(0) };
        fragment->set_level1id(header.level1id);
    }

    //stringstream msg;
    ////for(size_t i = (n32-1); i >= 0; --i)
    //msg << "============= INCOMING PACKET ==============";
    //log->info("{0} - {1}",__VTFUNC__,msg.str()); msg.str("");
    //for(size_t i = 0; i < n32; i++)
    //{
    //    msg << std::hex << (unsigned)m_data_in.at(i);
    //    log->info("{0} - {1}",__VTFUNC__, msg.str()); msg.str("");
    //}
    //msg.str("");
    //msg << "============================================";
    //log->info("{0} - {1}",__VTFUNC__,msg.str()); msg.str("");

    fragment->packet = m_data_in;

    if(!m_in_queue->try_enqueue(fragment))
    {
        log->warn("{0} - Unable to enqueue incoming data for Trigger ID {1:x}",__VTFUNC__,
            (unsigned)0x0);
    }
    if(continue_listening())
    {
        listen();
    }
    return;
}


} // namespace daq
} // namespace vts
