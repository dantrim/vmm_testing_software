//vts
#include "vts_test.h"
#include "helpers.h"
#include "tests/tests.h"

//std/stl
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
using namespace std;

//logging
#include "spdlog/spdlog.h"


namespace vts
{

VTSTest::VTSTest(QObject* parent) :
    QObject(parent)
{
    log = spdlog::get("vts_logger");
    m_fsm_state = vts::VTSTestState::TESTSTATEINVALID;
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

    connect(m_imp.get(), SIGNAL(finished()), this, SLOT(test_finished_slot()));

    // initialize the FSM but don't start it until the test has started
    bool status = m_imp->initialize(config);
    if(status)
    {
        update_fsm(vts::VTSTestState::INITIAL);
        emit test_ready();
    }

    return status;
}

vts::VTSTestState VTSTest::current_state()
{
    return m_fsm_state;
}

void VTSTest::update_fsm(vts::VTSTestState s)
{
    auto current = current_state();
    if(s == VTSTestState::INITIAL && (current != VTSTestState::TESTSTATEINVALID))
    {
        stringstream err;
        err << "Invalid FSM transition: " << VTSTestStateToStr(current) << " -> " << VTSTestStateToStr(s);
        throw std::runtime_error(err.str());
    }

    m_fsm_state = s;
    broadcast_state();
}

void VTSTest::initialize_fsm()
{

}

void VTSTest::state_updated_slot()
{
    stringstream msg;
    string new_state = current_state_name();
    msg << "Entering state \"" << new_state << "\"";
    log->info("{0} - {1}",__VTFUNC__,msg.str());
}

void VTSTest::test_finished_slot()
{
    stop();
}

string VTSTest::current_state_name()
{
    return VTSTestStateToStr(m_fsm_state);
}

void VTSTest::start()
{
    broadcast_state();
    log->info("{0} - {1}",__VTFUNC__,"Test starting...");
    update_fsm(vts::VTSTestState::RUNNING);
    m_imp->start();
    return;
}

void VTSTest::stop()
{
    log->info("{0} - {1}",__VTFUNC__,"Test stopping...");
    update_fsm(vts::VTSTestState::FINISHED);
}

void VTSTest::broadcast_state()
{
    QString qstate = QString::fromStdString(current_state_name());
    emit broadcast_state_signal(qstate);
}

} // namespace vts
