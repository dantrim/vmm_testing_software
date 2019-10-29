#include "vts_server.h"
#include <QHostAddress>
#include <QAbstractSocket>
#include <iostream>
#include <fstream>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLatin1String>
using namespace std;

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace vts
{

VTSServer::VTSServer(QWidget* parent)
{
    cout << "VTSServer::VTSServer()" << endl;
    m_server.listen(QHostAddress::LocalHost, 1234);
    connect(&m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

VTSServer::~VTSServer()
{
    cout << "VTSServer::~VTSServer()" << endl;
}

void VTSServer::onNewConnection()
{

    cout << "VTSServer::onNewConnection" << endl;
    QTcpSocket* clientSocket = m_server.nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    m_sockets.push_back(clientSocket);
    for(auto socket : m_sockets)
    {
        cout << clientSocket->peerAddress().toString().toStdString() << " connected to server!" << endl;
        socket->write(QByteArray::fromStdString(clientSocket->peerAddress().toString().toStdString() + " connected to server!\n"));
    }
}

void VTSServer::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{

    cout << "removing socket" << endl;
    if(socketState == QAbstractSocket::UnconnectedState)
    {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        m_sockets.removeOne(sender);
    }
}
std::string VTSServer::trim(const std::string& str,
                 const std::string& whitespace)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}
bool VTSServer::isASCII(const std::string& s)
{
    //https://www.fileformat.info/info/charset/UTF-8/list.htm
        return !std::any_of(s.begin(), s.end(), [](char c) { 
            return (static_cast<unsigned char>(c) > 127)
                    || (static_cast<unsigned char>(c) == 0x40)
                    || (static_cast<unsigned char>(c) == 0x60)
                    || (static_cast<unsigned char>(c) == 0x5e)
                    || (static_cast<unsigned char>(c) == 0x3f)
                    || (static_cast<unsigned char>(c) == 0x3e);
                    //|| (static_cast<unsigned char>(c) == 0x3d)
                   // || (static_cast<unsigned char>(c) == 0x3c)
                   // || (static_cast<unsigned char>(c) == 0x3b)
                   // || ((static_cast<unsigned char>(c) <= 0x2c) && (static_cast<unsigned char>(c) >= 0x23))
                   // || (static_cast<unsigned char>(c) == 0x21);
    });
}

std::string VTSServer::QByteArrayToValidString(QByteArray& json_byte_array)
{
    json_byte_array = json_byte_array.trimmed();
    QByteArray out;
    stringstream s;
    for(auto x : json_byte_array)
    {
        s.str("");
        if(!x) continue;
        s << x;
        if(!isASCII(s.str())) continue;
        out.append(x);
    }
    return out.toStdString();
}

void VTSServer::onReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());

    QByteArray data = sender->readAll();
    auto data_string = QByteArrayToValidString(data);
    try
    {
        auto json_input = json::parse(data_string);
        for(auto& element : json_input)
        {
            cout << "ITERATE: " << element << endl;
        }
        std::string dumped = json_input.dump();
        cout << "json size = " << json_input.size() << endl;
        cout << "================ JSON DUMP BEGIN ===============" << endl;
        cout << dumped << endl;
        cout << "================  JSON DUMP END  ===============" << endl;
        auto vec = json_input["peaking_time"];
        cout << "peaking_time size = " << vec.size() << endl;
        cout << "peaking times:" << endl;
        for(auto pt : vec)
        {
            cout << "   peaking time: " << pt << endl;
        }
        auto channels = json_input["channels"];
        cout << "channels size = " << channels.size() << endl;
        for(json::iterator chit = channels.begin(); chit != channels.end(); ++chit)
        {
            cout << "   channels[" << chit.key() << "] = " << chit.value() << endl;
        }
    } // try
    catch(std::exception& e)
    {
        stringstream err;
        err << "VTSServer failed to load command: " << e.what();
        cout << "JSON PARSING ERROR: " << err.str() << endl;
    } // catch
}


} // namespace vts
