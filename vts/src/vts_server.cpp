#include "vts_server.h"
#include "helpers.h"
#include <QCoreApplication>
#include <QHostAddress>
#include <QAbstractSocket>
#include <iostream>
#include <fstream>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLatin1String>
#include <string>
using namespace std;

#include "nlohmann/json.hpp"
using json = nlohmann::json;

//
#include "spdlog/spdlog.h"

// std
#include <memory>

namespace vts
{

VTSServer::VTSServer(QWidget* parent) :
    m_is_running(false),
    m_validate_json_string(false)
{
    log = spdlog::get("vts");
}

void VTSServer::load_config(json config_data)
{
    m_server_config = config_data;
}

bool VTSServer::start()
{
    log->info("Starting VTS server...");
    auto server_config = m_server_config.at("vts_server");
    std::string ip = server_config.at("server_ip");
    std::string jport = server_config.at("server_port");
    int port = std::stoi(jport);
    stringstream msg;
    msg << "VTS server hosted at (ip,port)=(" << ip << "," << port << ")";
    log->info("{0} - {1}", __VTFUNC__, msg.str());
    m_server.listen(QHostAddress(QString::fromStdString(ip)), port);
    //m_server.listen(QHostAddress::LocalHost,1234);
    connect(&m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    m_is_running = true;
    return true;
}


bool VTSServer::stop()
{
    log->info("Closing VTS server...");
    for(auto & conx : m_sockets)
        delete conx;
    m_server.close();
    QCoreApplication::quit();
    m_is_running = false;
    return true;
}

VTSServer::~VTSServer()
{
}

void VTSServer::onNewConnection()
{

    //log->debug("VTSServer::onNewConnection");
    QTcpSocket* clientSocket = m_server.nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    m_sockets.push_back(clientSocket);
   // for(auto socket : m_sockets)
   // {
   //     string msg = clientSocket->peerAddress().toString().toStdString() + " connected to server!";
   //     log->info("{0} - {1}", __VTFUNC__, msg);
   //     socket->write(QByteArray::fromStdString(clientSocket->peerAddress().toString().toStdString() + " connected to server!\n"));
   // }
}

void VTSServer::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{

//    string msg = "removing socket";
//    log->info("{0} - {1}", __VTFUNC__, msg);
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
    std::string data_string;
    if(m_validate_json_string)
    {
        data_string = QByteArrayToValidString(data);
    }
    else
    {
        data_string = data.toStdString();
    }

    log->critical("{0} - DATA RECEIVED : {1}", __VTFUNC__, data_string);

    //if(data_string=="EXIT")
    //{
    //    string msg = "received KILL command (->"+data_string+")";
    //    log->info("{0} - {1}", __VTFUNC__, msg);
    //    this->stop();
    //    return;
    //}

    try
    {
        auto message = json::parse(data_string);
        std::string message_string = message.dump();
        cout << "=================================" << endl;
        cout << "SERVER RECEIVED MESSAGE:" << endl;
        cout << message_string << endl;
        cout << "=================================" << endl;

        int cmd_id = int(message["ID"]);
        std::string message_type = message["TYPE"];
        bool send_reply = bool(message["EXPECTS_REPLY"]);
        auto message_data = message["DATA"];
        std::string message_data_string = message_data.dump();
        cout << "- - - - - - - - - - - - - - - - -" << endl;
        cout << "CMD:" << endl;
        cout << message_data_string << endl;
        cout << "=================================" << endl;

        /// catch server messages
        if(message_type == "SERVER")
        {
            // server type messages are single strings
            string server_cmd = message_data["CMD"];
            if(server_cmd == "EXIT")
            {
                string msg = "received KILL command";
                log->info("{0} - {1}", __VTFUNC__, msg);
                this->stop();
                return;
            }
        }
        else if(message_type == "TEST")
        {

        }


        /// done
        if(send_reply)
        {
            cout << "SERVER SENDING REPLY" << endl;
            json response = {
                {"CMD_ID", cmd_id},
                {"REPLY", true}
            };
            std::string response_str = response.dump();
            cout << "---> " << response_str << endl;
            QByteArray response_data;
            response_data.append(response_str.c_str());
            sender->write(response_data);
        }

    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "VTSServer failed to load command: " << e.what();
        cout << "JSON PARSING ERROR: " << err.str() << endl;
    } // catch

    //std::string response_str = response.dump();
    //cout << "BLAH " << response_str << endl;
    //QByteArray response_data;
    //response_data.append(response_str.c_str());
    //sender->write(response_data);

//    try
//    {
//        auto json_input = json::parse(data_string);
//        for(auto& element : json_input)
//        {
//            cout << "ITERATE: " << element << endl;
//        }
//        std::string dumped = json_input.dump();
//        cout << "json size = " << json_input.size() << endl;
//        cout << "================ JSON DUMP BEGIN ===============" << endl;
//        cout << dumped << endl;
//        cout << "================  JSON DUMP END  ===============" << endl;
//        auto vec = json_input["peaking_time"];
//        cout << "peaking_time size = " << vec.size() << endl;
//        cout << "peaking times:" << endl;
//        for(auto pt : vec)
//        {
//            cout << "   peaking time: " << pt << endl;
//        }
//        auto channels = json_input["channels"];
//        cout << "channels size = " << channels.size() << endl;
//        for(json::iterator chit = channels.begin(); chit != channels.end(); ++chit)
//        {
//            cout << "   channels[" << chit.key() << "] = " << chit.value() << endl;
//        }
//    } // try
//    catch(std::exception& e)
//    {
//        stringstream err;
//        err << "VTSServer failed to load command: " << e.what();
//        cout << "JSON PARSING ERROR: " << err.str() << endl;
//    } // catch
}


} // namespace vts
