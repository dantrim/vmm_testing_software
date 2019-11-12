#include "vts_server.h"
#include "communicator_frontend.h"
#include "helpers.h"
#include "vts_message.h"
#include "vts_commands.h"

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

VTSServer::VTSServer(QWidget* /*parent*/) :
    m_is_running(false),
    m_validate_json_string(false)
{
}

void VTSServer::load_config(json config_data)
{
    m_server_config = config_data;
}

bool VTSServer::start()
{
    log = spdlog::get("vts_logger");
    if(!log.get())
    {
        cout << "VTSServer::start    ERROR Failed to get registered logger" << endl;
        return false;
    }
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
    QTcpSocket* clientSocket = m_server.nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    m_sockets.push_back(clientSocket);
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
        auto raw_message = json::parse(data_string);
        vts::VTSMessage incoming(raw_message);
        VTSReply reply;

        cout << "=================================" << endl;
        cout << "VTS server :: Incoming Message: " << incoming.str() << endl;
        cout << "=================================" << endl;

        /////////////////////////////////////////////////////////////
        // handle incoming messages
        /////////////////////////////////////////////////////////////
        if(incoming.type() == vts::VTSMessageType::SERVER)
        {
            handle_server_command(incoming, reply);
        }
        else if(incoming.type() == vts::VTSMessageType::TEST)
        {
            handle_test_command(incoming, reply);
        }
        else if(incoming.type() == vts::VTSMessageType::FRONTEND)
        {
            handle_frontend_command(incoming, reply);
        }
        else
        {
            stringstream msg;
            msg << "Incoming message has unexpected message type: " << MessageTypeToStr(incoming.type()) << " (raw message: " << raw_message.dump() << ")";
            log->warn("{0} - {1}",__VTFUNC__,msg.str());
        }

        /////////////////////////////////////////////////////////////
        // send reply if one is requested and it has been filled
        /////////////////////////////////////////////////////////////
        if(incoming.expects_reply() && (reply.id()>=0))
        {
            sender->write(reply.byte_array());
        }

    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "VTS server failed to load command: " << e.what();
        log->error("{0} - {1}",__VTFUNC__,err.str());
    } // catch
}

void VTSServer::handle_server_command(const vts::VTSMessage& message,
                    vts::VTSReply& /*reply*/)
{
    log->info("{0} - {1}",__VTFUNC__,message.str());

    auto msg_data = message.data();
    string server_command = "";
    try
    {
        server_command = msg_data.at("CMD");
    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "Failed to acquire server command at message ID=" << message.id();
        err << ": " << e.what();
        log->error("{0} - {1}",__VTFUNC__,err.str());
        return;
    }
    if(server_command == "EXIT")
    {
        log->info("{0} - {1}",__VTFUNC__, "VTS received EXIT command");
        stop();
        return;
    }
}

void VTSServer::handle_test_command(const vts::VTSMessage& message,
                vts::VTSReply& reply)
{
    log->info("{0} - {1}",__VTFUNC__,message.str());
    auto msg_data = message.data();
    reply = vts::VTSReply(message.id(), msg_data);
}

void VTSServer::handle_frontend_command(const vts::VTSMessage& message,
                vts::VTSReply& reply)
{
    log->info("{0} - {1}",__VTFUNC__,message.str());
    auto msg_data = message.data();

    vts::CommunicatorFrontEnd* comm = new vts::CommunicatorFrontEnd();//m_server_config.at("frontend"));
    comm->load_config(m_server_config.at("frontend"));
    auto frontend_cmd = StrToCMDFrontEnd(msg_data.at("CMD"));
    if(frontend_cmd == CMDFrontEnd::PINGFPGA)
    {
        bool status = comm->ping_fpga();
        stringstream status_str;
        status_str << "Ping status? " << (status ? "GOOD" : "BAD");
        log->info("{0} - {1}",__VTFUNC__,status_str.str());

        status_str.str("");
        status_str << (status ? "OK" : "ERROR");
        json jreply = {
            {"STATUS", status_str.str() }
        };
        reply = vts::VTSReply(message.id(), jreply);
    }
    else if(frontend_cmd == CMDFrontEnd::CONFIGUREVMM || frontend_cmd == CMDFrontEnd::RESETVMM)
    {
        log->critical("{0} - {1}",__VTFUNC__,"HARDCODING DEFAULT VMM SPI JSON FILE");
        std::string vmm_file = "/Users/dantrim/workarea/NSW/vmm_testing/vmm_testing_software/vts/config/frontend/vmm_spi_default.json";

        bool perform_reset = (frontend_cmd == CMDFrontEnd::RESETVMM);

        bool status = comm->configure_vmm(vmm_file, perform_reset);

        stringstream status_str;
        status_str << (status ? "OK" : "ERROR");
        json jreply = {
            {"STATUS",status_str.str()}
        };
        reply = vts::VTSReply(message.id(), jreply);
    }
    else if(frontend_cmd == CMDFrontEnd::CONFIGUREFPGA)
    {
        log->critical("{0} - {1}",__VTFUNC__,"HARDCODING DEFAULT FPGA JSON FILE");
        std::string fpga_file = "/Users/dantrim/workarea/NSW/vmm_testing/vmm_testing_software/vts/config/frontend/fpga_register_default.json";
        bool status = comm->configure_fpga(fpga_file);

        stringstream status_str;
        status_str << (status ? "OK" : "ERROR");
        json jreply = {
            {"STATUS", status_str.str() }
        };
        reply = vts::VTSReply(message.id(), jreply);
    }
    else if(frontend_cmd == CMDFrontEnd::RESETFPGA)
    {
        bool status = comm->reset_fpga();
        stringstream status_str;
        status_str << (status ? "OK" : "ERROR");
        json jreply = {
            {"STATUS", status_str.str() }
        };
        reply = vts::VTSReply(message.id(), jreply);
    }
    else if(frontend_cmd == CMDFrontEnd::ACQON)
    {
        cout << "RECEIVED ACQON" << endl;
        bool status = comm->acq_toggle(1);
        stringstream status_str;
        status_str << (status ? "OK" : "ERROR");
        json jreply = {
            {"STATUS", status_str.str() }
        };
        reply = vts::VTSReply(message.id(), jreply);
    }
    else if(frontend_cmd == CMDFrontEnd::ACQOFF)
    {
        cout << "RECEIVED ACQOFF" << endl;
        bool status = comm->acq_toggle(0);
        stringstream status_str;
        status_str << (status ? "OK" : "ERROR");
        json jreply = {
            {"STATUS", status_str.str() }
        };
        reply = vts::VTSReply(message.id(), jreply);
    }

    delete comm;
}


} // namespace vts
