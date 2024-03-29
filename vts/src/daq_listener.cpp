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
            std::atomic<bool>& listen_flag)
{
    log = spdlog::get("vts_logger");

    m_active = false;
    m_listen_ip = listen_ip;
    m_listen_port = listen_port;

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
    m_start_time = std::chrono::system_clock::now();
    n_total_bytes = 0;
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
    log->trace("{0} - Shutting down listener of port {1}", __VTFUNC__, m_listen_port);
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
    auto current_time = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - m_start_time).count();
    int time_ms = 1;
    if(diff > time_ms)
    {
        float rate = (n_total_bytes / diff); // kB/s
        //rate *= 1000; // B/s
        n_total_bytes = 0;
        m_start_time = std::chrono::system_clock::now();
    }

    if(continue_listening())
    {
        m_socket->async_receive(boost::asio::buffer(m_in_buffer),
            boost::bind(&DataListener::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
}

void DataListener::handle_receive(const boost::system::error_code& /*error*/, size_t n_bytes)
{
    if(!continue_listening()) return;
    if(n_bytes == 0 && m_in_buffer.size() == 0)
    {
        listen();
    }
    n_total_bytes += n_bytes;


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
        VMMHeader header = *(VMMHeader*)(m_data_in.data());
        fragment->set_level1id(header.level1id);
    }

    fragment->packet = m_data_in;

    if(!m_in_queue->try_enqueue(fragment))
    {
        // more often than not, this failure occurs when we've shut down the data building at the end of the
        // run and since the data rate is  high, the latency between sending the stop command and
        // the time that the listener dies, we'll overflow the buffer... so just ignore this for now
        //log->warn("{0} - Unable to enqueue incoming data for Trigger ID {1:x}",__VTFUNC__,
        //    (unsigned)0x0);
        delete fragment;
    }
    if(continue_listening())
    {
        listen();
    }
    return;
}

} // namespace daq
} // namespace vts
