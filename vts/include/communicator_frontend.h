#ifndef VTS_COMMUNICATOR_FRONTEND_H
#define VTS_COMMUNICATOR_FRONTEND_H

// std/stl
#include <memory>

// json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

// logging
namespace spdlog {
    class logger;
}

namespace vts
{
    class CommunicatorFrontEnd
    {
        public :
            CommunicatorFrontEnd(json config);
            ~CommunicatorFrontEnd();

            bool power_board_toggle(bool turn_on);
            bool ping_fpga();
            bool reset_fpga();
            bool configure_fpga();
            bool acq_toggle(bool turn_on);
            bool reset_vmm();
            bool configure_vmm(std::string vmm_spi_file = "");

        private :
            std::shared_ptr<spdlog::logger> log;
            std::string m_board_ip;
            void load_config(json config);
    }; // class CommunicatorFrontEnd
} // namespace vts

#endif
