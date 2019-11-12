#ifndef VTS_COMMUNICATOR_FRONTEND_H
#define VTS_COMMUNICATOR_FRONTEND_H

// std/stl
#include <memory>

// json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

// Qt
#include <QObject>

// logging
namespace spdlog {
    class logger;
}

namespace vts
{
    class CommunicatorFrontEnd : QObject
    {
        Q_OBJECT

        public :
            explicit CommunicatorFrontEnd(QObject* parent =  0);
            ~CommunicatorFrontEnd();
            void load_config(json config);

            bool power_board_toggle(bool turn_on);
            bool ping_fpga();
            bool reset_fpga();
            bool configure_fpga();
            bool acq_toggle(bool turn_on);
            bool reset_vmm();
            bool configure_vmm(std::string vmm_spi_file = "", bool perform_reset = false);

        private :
            std::shared_ptr<spdlog::logger> log;
            std::string m_board_ip;
            int m_spi_recv_port;
    }; // class CommunicatorFrontEnd
} // namespace vts

#endif
