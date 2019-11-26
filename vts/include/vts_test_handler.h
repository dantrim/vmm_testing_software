#ifndef VTS_TEST_HANDLER_H
#define VTS_TEST_HANDLER_H

//vts
#include "vts_test.h"

//std/stl
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>

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
    class VTSTestHandler : public QObject
    {
        Q_OBJECT

        public :
            explicit VTSTestHandler(QObject* parent = 0);
            ~VTSTestHandler();

            // static checks before initialization
            static bool tests_are_ok(std::vector<std::string> test_config_files);
            static bool is_valid_test(std::string test_type);

            void load_output_config(const json& output_cfg);
            void load_frontend_config(const json& frontend_cfg, const json& daq_cfg);
            void load_test_config(const json& test_cfg);

            void start();
            bool is_running() { return m_is_running; }
            bool stop_all_tests()
            {
                return m_stop_all_tests.load(std::memory_order_acquire);
            } 

        private :
            bool m_is_running;
            std::shared_ptr<spdlog::logger> log;
            std::string m_vmm_serial_id;
            json m_output_cfg;
            json m_frontend_cfg;
            json m_daq_cfg;
            std::vector<std::string> m_test_names;
            std::vector<std::string> m_test_configs;
            std::shared_ptr<vts::VTSTest> m_test;
            std::thread m_test_thread;
            std::atomic<bool> m_stop_all_tests;
            

        signals :
            void signal_test_status_update(float);
            void signal_stop_current_test();
            void tests_finished();

        public slots :
            void update_state(QString);
            void test_status_update_slot(float);
            void stop();
            
    }; // class VTSTestHandler

} // namespace vts


#endif
