#include "tester.h"

//std/stl
#include <iostream>
#include <vector>
using namespace std;

//Qt
#include <QDebug>
#include <QAbstractState>

VMMTester::VMMTester(QObject* parent) :
    QObject(parent)
{
    cout << "VMMTester()" << endl;
}
VMMTester::~VMMTester()
{
    cout << "~VMMTester()" << endl;
}
void VMMTester::initialize(string name)
{
    m_name = name;
    cout << "VMMTester::initialize    name = " << name << endl;

    // construct the FSM
    fsm = new QStateMachine();
    state_Ready = new QState();
    state_prepareConfiguration = new QState();
    state_Configured = new QState();
    state_CollectingData = new QState();
    state_Analyzing = new QState();
    state_TestComplete = new QFinalState();
    vector<QAbstractState*> states = { state_Ready, state_prepareConfiguration, state_Configured, state_CollectingData, state_Analyzing , state_TestComplete };

    // add the states
    fsm->addState(state_Ready);
    fsm->addState(state_prepareConfiguration);
    fsm->addState(state_Configured);
    fsm->addState(state_CollectingData);
    fsm->addState(state_Analyzing);
    fsm->addState(state_TestComplete);
    fsm->setInitialState(state_Ready);

    // FSM
    state_Ready->addTransition(this, SIGNAL(begin_test()),
        state_prepareConfiguration);
    state_prepareConfiguration->addTransition(this, SIGNAL(begin_test()),
        state_Configured);
    state_Configured->addTransition(this, SIGNAL(begin_test()),
        state_CollectingData);
    state_CollectingData->addTransition(this, SIGNAL(begin_test()),
        state_Analyzing);
    state_Analyzing->addTransition(this, SIGNAL(begin_test()),
        state_TestComplete);
    connect(this, SIGNAL(test_done()), this, SLOT(test_completed_slot()));

    connect(fsm, SIGNAL(finished()), this, SLOT(test_completed_slot()));

    fsm->start();

    for(const auto& state : states)
    {
        connect(state, SIGNAL(entered()), this, SLOT(state_updated_slot()));
    }
//    connect(state_Ready, SIGNAL(entered()), this, SLOT(state_updated_slot()));
//    connect(state_prepareConfiguration, SIGNAL(entered()), this, SLOT(state_updated_slot()));
//    connect(state_Configured, SIGNAL(entered()), this, SLOT(state_updated_slot()));
}

void VMMTester::PrepareForConfigure

std::string VMMTester::currentStateStr() const
{
    QSet<QAbstractState*> states = fsm->configuration();
    string state_str = "";
    for(auto & itr : states)
    {
        if(itr == state_Ready) { state_str = "Ready"; }
        else if(itr == state_prepareConfiguration) { state_str = "Preparing Configuration"; }
        else if(itr == state_Configured) { state_str = "Configured"; }
        else if(itr == state_CollectingData) { state_str = "Collecting Data"; }
        else if(itr == state_Analyzing) { state_str = "Analyzing Data"; }
        else if(itr == state_TestComplete) { state_str = "Test Complete"; }
        else {
            state_str = "Unknown";
        }
        break;
    }
    return state_str;
}

void VMMTester::state_updated_slot()
{
    QString qstate = QString::fromStdString(currentStateStr());
    emit state_updated(qstate);
}

void VMMTester::getState()
{
    cout << "VMMTester::getState    Current state = " << currentStateStr() << endl;
}

void VMMTester::begin_test_slot()
{
    emit begin_test();
}

void VMMTester::test_completed_slot()
{
    emit test_completed();
}

void VMMTester::end_test()
{

}
