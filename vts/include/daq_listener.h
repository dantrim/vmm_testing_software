#ifndef VTS_DAQ_LISTENER_H
#define VTS_DAQ_LISTENER_H

/////////////////////////////////////////////////////
//
// daq_listener.h
//
// structure that instantiates a UDP socket listening
// for data from the frontend and queueing it
//
// daniel joseph antrim
// october 2019
// daniel.joseph.antrim@cern.ch
//
/////////////////////////////////////////////////////

//vts
#include "daq_defs.h"
#include "daq_data_fragment.h"

//std/stl
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

//boost
#include <boost/asio.hpp>

//logging
namespace spdlog {
    class logger;
}

namespace vts
{
namespace daq
{

    class DataListener
    {
        public :
            DataListener(uint32_t listen_ip, uint32_t listen_port,
                        std::shared_ptr<boost::asio::io_service> io_service,
                        DataQueue* input_queue,
                        std::atomic<bool> & listen_flag);
            virtual ~DataListener(){};

            bool connect();
            void start();
            void shutdown();
            bool continue_listening();
            bool active() { return m_active; }
            virtual void listen();
            virtual void stop();
            virtual void handle_receive(const boost::system::error_code& error,
                                                                std::size_t n_bytes);

        private :

            bool m_active;
            std::shared_ptr<spdlog::logger> log;
            std::atomic<bool>* m_listen_flag;

            std::chrono::system_clock::time_point m_start_time;
            int n_total_bytes;

            uint32_t m_listen_ip;
            uint32_t m_listen_port;

            DataQueue* m_in_queue;
            std::thread m_thread;

            std::vector<uint32_t> m_data_in;
            data_array_t m_in_buffer;

            boost::asio::ip::udp::endpoint m_udp_endpoint;
            std::shared_ptr<boost::asio::io_service> m_io_service;
            std::shared_ptr<boost::asio::ip::udp::socket> m_socket;



    }; // class DataListener

} // namespace daq
} // namespace vts

#endif
