#ifndef VTS_DAQ_HANDLER_H
#define VTS_DAQ_HANDLER_H

//vts
#include "daq_defs.h"
#include "daq_listener.h"

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//logging
namespace spdlog {
    class logger;
}

//boost
#include <boost/asio.hpp>

//std/stl
#include <atomic>

//Qt
#include <QObject>

namespace vts
{
namespace daq
{

    class DaqHandler : public QObject
    {
        Q_OBJECT

        public :
            explicit DaqHandler(QObject* parent = 0);
            virtual ~DaqHandler(){};

            void load_connections(const json& frontend_config, const json& daq_config);
            std::vector< DataQueue* > listen_queues() { return m_listen_queues; }

            bool is_running() { return m_is_running; }

        private :
            std::shared_ptr<spdlog::logger> log;
            std::shared_ptr<boost::asio::io_service> m_io_service;

            uint32_t m_listen_ip;
            uint32_t m_listen_port;

            std::vector< DataQueue* > m_listen_queues;
            std::vector< DataListener* > m_listeners;

            // flags
            bool m_is_running;
            std::atomic<bool> m_listen_flag;
            std::atomic<bool> m_build_flag;

        public slots :
            void start_listening();
            void stop_listening();


    }; // class DaqHandler

} // namespace daq
} // namespace vts

#endif
