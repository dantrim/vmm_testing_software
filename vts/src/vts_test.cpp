//vts
#include "vts_test.h"
#include "helpers.h"
#include "tests/tests.h"

//std/stl
#include <string>
#include <sstream>
using namespace std;

//logging
#include "spdlog/spdlog.h"


namespace vts
{

VTSTest::VTSTest(QObject* parent) :
    QObject(parent)
{
    log = spdlog::get("vts_logger");
}
VTSTest::~VTSTest()
{
}

bool VTSTest::initialize(const json& config)
{
    stringstream msg;
    msg << "Initializing test: " << config.dump();
    log->info("{0} - {1}",__VTFUNC__,msg.str());

    string test_type = config.at("test_type").get<std::string>();
    if(test_type == "PassThrough")
    {
        m_imp = std::make_shared<vts::VTSTestPassThrough>();
    }
    else
    {
        msg.str("");
        msg << "Unhandled test type encountered (=\"" << test_type << "\")";
        log->error("{0} - {1}",__VTFUNC__,msg.str());
        return false;
    }
    return true;

}

void VTSTest::start()
{
    log->info("{0} - {1}",__VTFUNC__,"Test starting...");
    return;
}

} // namespace vts
