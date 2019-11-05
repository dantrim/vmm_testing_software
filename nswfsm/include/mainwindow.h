#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//test
//#include "tester.h"
#include "new_test.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

        void connect_widgets();
    
    private:
        Ui::MainWindow *ui;
        //VMMTester* tester;
        NewTest* tester;
        int test_number;

    public slots :
        void initialize_tester();
        void state_updated_slot(QString);
        void test_completed_slot();

};
#endif // MAINWINDOW_H
