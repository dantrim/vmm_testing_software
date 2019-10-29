#include <QCoreApplication>
#include <iostream>
using namespace std;

#include "vts_server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    cout << "Running application" << endl;
    cout << "Starting VTS..." << endl;
    vts::VTSServer* server = new vts::VTSServer();

    return a.exec();
}
