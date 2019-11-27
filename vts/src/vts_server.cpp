#include "vts_server.h"
#include "communicator_frontend.h"
#include "helpers.h"
#include "vts_message.h"
#include "vts_commands.h"
#include "vts_test_types.h"

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
#include <future>
using namespace std;

#include "nlohmann/json.hpp"
using json = nlohmann::json;

// loggin
#include "spdlog/spdlog.h"

// std
#include <memory>
#include <chrono>
#include <vector>

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
    connect(&m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()), Qt::DirectConnection);
    m_is_running = true;


    return true;
}


bool VTSServer::stop()
{
    log->info("Closing VTS server...");
    //for(auto & conx : m_sockets)
    //    delete conx;
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
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::DirectConnection);
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    m_sockets.push_back(clientSocket);
}

void VTSServer::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{

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

    log->debug("{0} - DATA RECEIVED : {1}", __VTFUNC__, data_string);
    try
    {
        auto raw_message = json::parse(data_string);
        vts::VTSMessage incoming(raw_message);
        VTSReply reply;

        //cout << "=================================" << endl;
        //cout << "VTS server :: Incoming Message: " << incoming.str() << endl;
        //cout << "=================================" << endl;

        /////////////////////////////////////////////////////////////
        // handle incoming messages
        /////////////////////////////////////////////////////////////
        if(incoming.type() == vts::VTSMessageType::SERVER)
        {
            handle_server_command(incoming, reply);
        }
        else if(incoming.type() == vts::VTSMessageType::VMMTEST)
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
    auto msg_data = message.data();
    reply = vts::VTSReply(message.id(), msg_data);

    auto test_cmd = StrToCMDVMMTest(msg_data.at("CMD"));
    auto test_data = msg_data.at("TEST_DATA");

    stringstream msg;
    bool ok = true;
    if(test_cmd == CMDVMMTest::LOAD)
    {
        if(m_test_handler.get() != nullptr)
        {
            if(!m_test_handler->is_running())
            {
                m_test_handler.reset();
            }
            else
            {
                log->error("{0} - {1}",__VTFUNC__,"TestHandler already started, cannot load new tests!");
                ok = false;
            }
        }

        if(!vts::VTSTestHandler::tests_are_ok(test_data.at("TEST_CONFIG"))) { ok = false; }

        if(ok)
        {
            m_test_handler = std::make_shared<vts::VTSTestHandler>();
            m_test_handler->load_output_config(m_server_config.at("test_output"));
            m_test_handler->load_frontend_config(m_server_config.at("frontend"), m_server_config.at("daq"));
            m_test_handler->load_test_config(test_data);
            connect(this, SIGNAL(signal_stop_all_tests()),
                    m_test_handler.get(), SLOT(stop()), Qt::DirectConnection);
            connect(m_test_handler.get(), SIGNAL(tests_finished()), this, SLOT(tests_finished()), Qt::DirectConnection);
        }
    }
    else if(test_cmd == CMDVMMTest::START)
    {
        if(m_test_handler.get() == nullptr)
        {
            log->error("{0} - {1}",__VTFUNC__,"TestHandler not initialized, cannot start tests!");
            ok = false;
        }

        if(ok)
        {
            m_thread = std::thread(
                [this] { m_test_handler->run(); }
            );
        }
    }
    else if(test_cmd == CMDVMMTest::STOP)
    {
        if(m_test_handler.get() == nullptr)
        {
            log->error("{0} - {1}",__VTFUNC__,"TestHandler not started, no tests to stop!");
            ok = false;
        }

        if(ok)
        {
            m_test_handler->stop();
            if(m_thread.joinable())
            {
                m_thread.join();
            }
        }
    }

    stringstream status_str;
    status_str << (ok ? "OK" : "ERROR");
    json jreply = {
        {"STATUS", status_str.str() }
    };
    reply = vts::VTSReply(message.id(), jreply);
}

void VTSServer::tests_finished()
{
    if(m_test_handler.get())
    {
        m_test_handler->stop();
    }
    stringstream msg;
    msg << "====================================";
    log->info("{0} - {1}",__VTFUNC__,msg.str()); msg.str("");
    msg << "ALL TESTS COMPLETED";
    log->info("{0} - {1}",__VTFUNC__,msg.str()); msg.str("");
    msg << "====================================";
    log->info("{0} - {1}",__VTFUNC__,msg.str()); msg.str("");
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
