//vts
#include "vts_test.h"
#include "helpers.h"
#include "tests/tests.h"

//std/stl
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <future>
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

int VTSTest::current_step()
{
    return m_imp->get_current_state();
}

int VTSTest::n_steps()
{
    return m_imp->get_n_states();
}

bool VTSTest::initialize(const json& config, const json& frontend_cfg)
{
    update_fsm(VTSTestState::NONE);

    stringstream msg;
    msg << "Initializing test: " << config.dump();
    log->info("{0} - {1}",__VTFUNC__,msg.str());

    string test_type = config.at("test_type").get<std::string>();
    if(test_type == "PassThrough")
    {
        m_imp = std::make_shared<vts::VTSTestPassThrough>();
        m_imp->load_test_config(config, frontend_cfg);
    }
    else
    {
        msg.str("");
        msg << "Unhandled test type encountered (=\"" << test_type << "\")";
        log->error("{0} - {1}",__VTFUNC__,msg.str());
        return false;
    }


    update_fsm(VTSTestState::INITIALIZING);
    // initialize the FSM but don't start it until the test has started
    bool initialize_status = m_imp->initialize(config);
    if(initialize_status)
    {
        emit test_ready();
        update_fsm(vts::VTSTestState::INITIAL);
    }
    else
    {
        m_imp.reset();
        update_fsm(VTSTestState::NONE);
    }

    connect(m_imp.get(), SIGNAL(finished()), this, SLOT(test_finished_slot()));

    return initialize_status;
}

bool VTSTest::load()
{
    update_fsm(VTSTestState::LOADING);
    bool load_status = m_imp->load();
    if(load_status)
    {
        update_fsm(VTSTestState::READY);
    }
    return load_status;
}

bool VTSTest::configure()
{
    return m_imp->configure();
}

bool VTSTest::run()
{
    return m_imp->run();
}

bool VTSTest::analyze()
{
    return m_imp->analyze();
}

bool VTSTest::analyze_test()
{
    return m_imp->analyze_test();
}

bool VTSTest::finalize()
{
    update_fsm(VTSTestState::FINALIZING);
    bool status = m_imp->finalize();
    if(status)
    {
        update_fsm(VTSTestState::FINISHED);
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
    if(s == VTSTestState::INITIALIZING && (current != VTSTestState::NONE))
    {
        stringstream err;
    }

    if(s == VTSTestState::INITIAL && (current != VTSTestState::INITIALIZING))
    {
        stringstream err;
        err << "Invalid FSM transition: " << transition_string(current, s);
        throw std::runtime_error(err.str());
    }

    m_fsm_state = s;
    broadcast_state();
}

string VTSTest::transition_string(VTSTestState current, VTSTestState next)
{
    stringstream s;
    s << VTSTestStateToStr(current) << " -> " << VTSTestStateToStr(next);
    return s.str();
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
