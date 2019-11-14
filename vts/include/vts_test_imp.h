#ifndef VTS_TEST_IMP_H
#define VTS_TEST_IMP_H

//Qt
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

class VTSTestImp : public QObject
{
    Q_OBJECT

    public :
        virtual bool initialize(const json& config) = 0;
        virtual bool start() = 0;

    protected :
        std::shared_ptr<spdlog::logger> log;

    signals :
        void finished();

}; // class VTSTestImp

} // namespace vts

#endif
