#ifndef TESTER_H
#define TESTER_H

// Qt
#include <QtCore>
#include <QStateMachine>
#include <QState>

//std/stl
#include <string>

class VMMTester : public QObject
{
    Q_OBJECT

    public :
        explicit VMMTester(QObject* parent = 0);
        ~VMMTester();

        void initialize(std::string name);
        std::string name() const { return m_name; }
        std::string currentStateStr() const;

    private :

        std::string m_name;
        QStateMachine* fsm;
        QState* state_Ready;
        QState* state_prepareConfiguration;
        QState* state_Configured;
        QState* state_CollectingData;
        QState* state_Analyzing;
        QFinalState* state_TestComplete;

    public slots :

        void begin_test_slot();

        void getState();
        void state_updated_slot();
        void test_completed_slot();
        void end_test();

    signals :
        void begin_test();
        void state_updated(QString);
        void test_completed();
        void test_done();
        

}; // class VMMTester

#endif
