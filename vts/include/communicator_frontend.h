#ifndef VTS_COMMUNICATOR_FRONTEND_H
#define VTS_COMMUNICATOR_FRONTEND_H

// std/stl
#include <memory>
#include <vector>

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
            bool configure_fpga(std::string fpga_file = "");
            bool configure_fpga(json trigger_conf, json clock_conf);
            bool acq_toggle(bool turn_on);
            bool reset_vmm();
            bool configure_vmm(std::string vmm_spi_file = "", bool perform_reset = false);
            bool configure_vmm(json vmm_spi, bool perform_reset = false);
            void construct_spi(QDataStream& stream,
                            const std::vector<std::string>& global,
                            const std::vector<std::string>& channel);
            bool sample_xadc(int n_samples = 1000, int sampling_delay = 10000 /*in 5 ns steps*/);
            void write(const QByteArray& data, int port);

        private :
            std::shared_ptr<spdlog::logger> log;
            std::string m_board_ip;
            int m_spi_recv_port;
            int m_fpga_recv_port;
            int m_xadc_recv_port;
    }; // class CommunicatorFrontEnd
} // namespace vts

#endif
