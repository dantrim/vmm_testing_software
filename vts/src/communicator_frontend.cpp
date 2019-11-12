// vts
#include "helpers.h"
#include "communicator_frontend.h"
#include "spi_builder.h"

//std/stl
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

// logging
#include "spdlog/spdlog.h"

// Qt
#include "QUdpSocket"
#include "QDataStream"
#include "QByteArray"
//#include <QProcess>

namespace vts
{

//CommunicatorFrontEnd::CommunicatorFrontEnd(json config)
CommunicatorFrontEnd::CommunicatorFrontEnd(QObject* parent) :
    QObject(parent),
    m_board_ip(""),
    m_spi_recv_port(0)
{
}

CommunicatorFrontEnd::~CommunicatorFrontEnd()
{
}

void CommunicatorFrontEnd::load_config(json config)
{
    try
    {
        m_board_ip = config.at("board_ip");
        m_spi_recv_port = std::stoi(config.at("spi_port").get<std::string>());
    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "Problem loading front-end configuration data: " << e.what();
        throw std::runtime_error(err.str());
    }
}

bool CommunicatorFrontEnd::power_board_toggle(bool turn_on)
{
    if(turn_on)
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning frontend ON");
    }
    else
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning frontend OFF");
    }
    return true;
}

bool CommunicatorFrontEnd::ping_fpga()
{
    stringstream msg;
    msg << "Pinging FPGA at IP: " << m_board_ip;
    log->info("{0} - {1}",__VTFUNC__,msg.str());

    msg.str("");
    msg << "ping " << m_board_ip << " -c 1 -W 1 > /dev/null";
    int status_code = system(msg.str().c_str());
    if(status_code != 0)
        return false;
    return true;
}

bool CommunicatorFrontEnd::configure_fpga()
{
    log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Configuring FPGA");
    return true;
}

bool CommunicatorFrontEnd::acq_toggle(bool turn_on)
{
    if(turn_on)
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning ACQ ON");
    }
    else
    {
        log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Turning ACQ OFF");
    }
    return true;
}

bool CommunicatorFrontEnd::reset_fpga()
{
    log->info("{0} - {1}",__VTFUNC__,"(NOT IMPLEMENTED) Resetting FPGA");
    return true;
}

bool CommunicatorFrontEnd::configure_vmm(std::string spi_filename, bool perform_reset)
{
    if(spi_filename == "")
        return false;

    json spi_json;
    try
    {
        std::ifstream ifs(spi_filename);
        spi_json = json::parse(ifs);
    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "Could not load VMM SPI json file: " << e.what();
        log->error("{0} - {1}",__VTFUNC__,err.str());
        return false;
    }


    //
    // Get the 32-bit words
    //
    json vmm_top_lvl = spi_json.at("vmm_spi");
    vector<string> global_vec =
        vts::spi::spi_global_register_vec(
            vmm_top_lvl.at("global_registers"),
            perform_reset
        );
    vector<string> channel_vec =
        vts::spi::spi_channel_register_vec(vmm_top_lvl.at("channel_registers"));

    //
    // build the datagram to send
    //
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.device()->seek(0);

    bool ok;

    // header information hard coded by firmware, it has no meaning
    uint16_t vmm_mask = 0x1; // assume there is always only a singe VMM
    QString h1 = "ffbb";
    QString h2 = "aadd";
    QString h3 = "bb00";
    out << (quint16)h1.toUInt(&ok,16)
        << (quint16)h2.toUInt(&ok,16)
        << (quint16)vmm_mask
        << (quint16)h3.toUInt(&ok,16);

    // write global (first block)
    out << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(5))).toUInt(&ok,2));
    out << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(4))).toUInt(&ok,2));
    out << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(3))).toUInt(&ok,2));

    // write channels
    for(int i = 63; i >=0; i--)
    {
        QString first8bits = QString::fromStdString(channel_vec.at(i)).mid(8,8);
        QString second16bits = QString::fromStdString(channel_vec.at(i)).mid(16,16);

        out << (quint8)(first8bits).toUInt(&ok,2);
        out << (quint16)(second16bits).toUInt(&ok,2);
    } // i

    // write global (second block)
    out << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(2))).toUInt(&ok,2));
    out << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(1))).toUInt(&ok,2));
    out << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(0))).toUInt(&ok,2));

    // prepare the socket and write
    QUdpSocket* socket = new QUdpSocket(this);
    QString ip_qstring = QString::fromStdString(m_board_ip);
    auto ip = QHostAddress(ip_qstring).toIPv4Address();
    QHostAddress send_to_address = QHostAddress(ip);
    socket->writeDatagram(datagram, send_to_address, m_spi_recv_port);

    return true;
}

} // namespace vts
