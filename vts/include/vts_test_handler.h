#ifndef VTS_TEST_HANDLER_H
#define VTS_TEST_HANDLER_H

//std/stl
#include <string>
#include <vector>
#include <map>
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
    class VTSTestHandler : QObject
    {
        public :
            explicit VTSTestHandler(QObject* parent = 0);
            ~VTSTestHandler();

            // static checks before initialization
            static bool tests_are_ok(std::vector<std::string> test_config_files);
            static bool is_valid_test(std::string test_type);

            void load_test_configs(std::vector<std::string> test_config_files);

            void start();
            void stop();

        private :
            std::shared_ptr<spdlog::logger> log;
            std::vector<std::string> m_test_configs;
            std::map<std::string, std::string> m_test_config_map;
            
    }; // class VTSTestHandler

} // namespace vts


#endif
