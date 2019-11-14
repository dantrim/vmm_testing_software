#ifndef VTS_TEST_H
#define VTS_TEST_H

// std/stl
#include <memory>

// vts
namespace vts {
    class VTSTestImp;
}

// Qt
#include <QObject>

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//logging
namespace spdlog {
    class logger;
}

namespace vts
{

class VTSTest : QObject
{
    Q_OBJECT

    public :
        explicit VTSTest(QObject* parent =  0);
        ~VTSTest();

        bool initialize(const json& test_config);

        void start();

    private :
        std::shared_ptr<spdlog::logger> log;
        std::shared_ptr<vts::VTSTestImp> m_imp;

}; // class VTSTest

} // namespace vts

#endif
