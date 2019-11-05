#ifndef NEW_TEST_H
#define NEW_TEST_H

// tester
#include "tester.h"

class NewTest : public VMMTester
{
    Q_OBJECT

    public :
        explicit NewTest();//QObject* parent = 0);
        ~NewTest();
        void test_signal();
    public slots :
        void new_test_slot();

}; // class NewTest

#endif
