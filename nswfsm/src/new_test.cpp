#include "new_test.h"

//std/stl
#include <iostream>
using namespace std;

NewTest::NewTest()//QObject* parent) :
//    QObject(parent)
{
    cout << "NewTest()" << endl;
}
NewTest::~NewTest()
{
    cout << "~NewTest()" << endl;
}

void NewTest::new_test_slot()
{
    cout << "NewTest::new_test_slot()" << endl;
}
