#include "mainwindow.h"
#include "ui_mainwindow.h"

//std/stl
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , test_number(0)
    
{
    ui->setupUi(this);
    connect_widgets();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connect_widgets()
{
    connect(ui->button_init_test, SIGNAL(clicked()),
                this, SLOT(initialize_tester()));
}

void MainWindow::initialize_tester()
{
    cout << "MainWindow::initialize_tester()" << endl;
    if(tester)
    {
        cout << "MainWindow::initialize_tester    ERROR Tester is non-null!" << endl;
        return;
    }
    //tester = new VMMTester();
    tester = new NewTest();
    connect(ui->button_getState, SIGNAL(clicked()),
                tester, SLOT(new_test_slot()));

    connect(ui->button_begin_test, SIGNAL(clicked()),
                tester, SLOT(begin_test_slot()));
    connect(ui->button_getState, SIGNAL(clicked()),
                tester, SLOT(getState()));
    connect(tester, SIGNAL(state_updated(QString)),
            this, SLOT(state_updated_slot(QString)));
    connect(tester, SIGNAL(test_completed()),
            this, SLOT(test_completed_slot()));
    connect(ui->button_end_test, SIGNAL(clicked()),
            tester, SLOT(end_test()));

    stringstream test_name;
    test_name << "TestBaselines_" << test_number;
    
    tester->initialize(test_name.str());
}

void MainWindow::state_updated_slot(QString state)
{
    ui->label_FSMState->setText(state);
}

void MainWindow::test_completed_slot()
{
    cout << "MainWindow::test_completed_slot   Test \"" << tester->name() << "\" has completed!" << endl;
    test_number++;
    delete tester;
    tester = 0x0;
}
