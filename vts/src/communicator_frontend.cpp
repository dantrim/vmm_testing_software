// vts
#include "helpers.h"
#include "communicator_frontend.h"
#include "spi_builder.h"
#include "frontend_address.h"

//std/stl
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
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
    m_spi_recv_port(0),
    m_fpga_recv_port(0)
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
        m_fpga_recv_port = std::stoi(config.at("fpga_port").get<std::string>());
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

void CommunicatorFrontEnd::write(const QByteArray& data, int port)
{
    QUdpSocket* socket = new QUdpSocket(this);
    QString ip_qstring = QString::fromStdString(m_board_ip);
    auto ip = QHostAddress(ip_qstring).toIPv4Address();
    socket->writeDatagram(data, QHostAddress(ip), port);
    delete socket;
}

bool CommunicatorFrontEnd::configure_fpga(string fpga_config_file)
{
    if(fpga_config_file == "")
        return false;

    json fpga_json;
    try
    {
        std::ifstream ifs(fpga_config_file);
        fpga_json = json::parse(ifs);
    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "Could not load FPGA CONFIG json file: " << e.what();
        log->error("{0} - {1}",__VTFUNC__,err.str());
        return false;
    }

    json fpga_top_lvl = fpga_json.at("fpga_registers");
    json fpga_trigger = fpga_top_lvl.at("trigger");
    json fpga_clocks = fpga_top_lvl.at("clocks");

    //
    // start building the words
    //

    bool ok;
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.device()->seek(0);

    QString dummy_header0 = "CCAA";
    QString dummy_header1 = "AAFF";

    // header
    out << (quint32)(dummy_header0.toUInt(&ok,16))
        << (quint32)(dummy_header1.toUInt(&ok,16));

    // trigger mode
    std::map<std::string, int> trig_mode_map = {{"pulser",7},{"external",4}};
    out << (quint32)(FPGARegister::TRIGGERMODE)
        << (quint32)(trig_mode_map.at(fpga_trigger.at("run_mode")));

    // trigger latency
    out << (quint32)(FPGARegister::LATENCY)
        << (quint32)(std::stoi(fpga_trigger.at("latency").get<std::string>()));

    // readout mode (fixed TAC/BC latency or normal)
    // assuming always false
    out << (quint32)(FPGARegister::READOUTMODE)
        << (quint32)(0);

    // ckbc max number (number of CKBCs for fixed TAC mode)
    out << (quint32)(FPGARegister::CKBCMAXNUMBER)
        << (quint32)(std::stoi(fpga_trigger.at("ckbc_max_number").get<std::string>()));

    // trigger dead time
    out << (quint32)(FPGARegister::TRIGGERDEADTIME)
        << (quint32)(std::stoi(fpga_trigger.at("trigger_dead_time").get<std::string>()));

    // art timeout
    out << (quint32)(FPGARegister::ARTTIMEOUT)
        << (quint32)(std::stoi(fpga_trigger.at("art_timeout").get<std::string>()));

    // latency extra ckbc
    out << (quint32)(FPGARegister::LATENCYEXTRACKBC)
        << (quint32)(std::stoi(fpga_trigger.at("latency_extra_ckbc").get<std::string>()));

    //
    // prepare socket and write
    //
    write(datagram, m_fpga_recv_port);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    datagram.clear();
    out.device()->seek(0);

    // header
    out << (quint32)(dummy_header0.toUInt(&ok,16))
        << (quint32)(dummy_header1.toUInt(&ok,16));

    // cktk max number
    out << (quint32)(FPGARegister::CKTKMAXNUMBER)
        << (quint32)(std::stoi(fpga_clocks.at("cktk_max_number").get<std::string>()));

    // ckbc frequency
    out << (quint32)(FPGARegister::CKBCFREQUENCY)
        << (quint32)(std::stoi(fpga_clocks.at("ckbc").get<std::string>()));

    // cktp max number
    QString infinite_pulses = "FFFF";
    int32_t n_pulses = std::stoi(fpga_clocks.at("cktp_max_number").get<std::string>());
    out << (quint32)(FPGARegister::CKTPMAXNUMBER);
    if(n_pulses<0)
        out << (quint32)(infinite_pulses.toUInt(&ok,16));
    else
        out << (quint32)(std::stoi(fpga_clocks.at("cktp_max_number").get<std::string>()));

    // cktp skew
    out << (quint32)(FPGARegister::CKTPSKEW)
        << (quint32)(std::stoi(fpga_clocks.at("cktp_skew").get<std::string>()));

    // cktp period
    out << (quint32)(FPGARegister::CKTPPERIOD)
        << (quint32)(std::stoi(fpga_clocks.at("cktp_period").get<std::string>()));

    // cktp width
    out << (quint32)(FPGARegister::CKTPWIDTH)
        << (quint32)(std::stoi(fpga_clocks.at("cktp_width").get<std::string>()));

    //
    // prepare socket and write
    //
    write(datagram, m_fpga_recv_port);

    return true;
}


bool CommunicatorFrontEnd::acq_toggle(bool turn_on)
{
    bool ok;
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.device()->seek(0);

    QString dummy_header0 = "CCAA";
    QString dummy_header1 = "AAFF";

    // header
    out << (quint32)(dummy_header0.toUInt(&ok,16))
        << (quint32)(dummy_header1.toUInt(&ok,16));

    // acq on/off
    out << (quint32)(FPGARegister::DAQMODE);
    if(turn_on)
    {
        out << (quint32)(0x1);
    }
    else
    {
        out << (quint32)(0x0);
    }

    //
    // prepare socket and write
    write(datagram, m_fpga_recv_port);


    return true;
}

bool CommunicatorFrontEnd::reset_fpga()
{

    bool ok;
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.device()->seek(0);

    QString dummy_header0 = "CCAA";
    QString dummy_header1 = "AAFF";

    // header
    out << (quint32)(dummy_header0.toUInt(&ok,16))
        << (quint32)(dummy_header1.toUInt(&ok,16));

    // reset
    out << (quint32)(FPGARegister::FPGARESET)
        << (quint32)(0xAA);

    // prepare and write
    write(datagram, m_fpga_recv_port);

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

    construct_spi(out, global_vec, channel_vec);

    // prepare the socket and write
    write(datagram, m_spi_recv_port);

    return true;

}

void CommunicatorFrontEnd::construct_spi(QDataStream& stream,
            const std::vector<string>& global_vec,
            const std::vector<string>& channel_vec)
{

    bool ok;

    // header information hard coded by firmware, it has no meaning
    uint16_t vmm_mask = 0x1; // assume there is always only a singe VMM
    QString h1 = "ffbb";
    QString h2 = "aadd";
    QString h3 = "bb00";
    stream << (quint16)h1.toUInt(&ok,16)
        << (quint16)h2.toUInt(&ok,16)
        << (quint16)vmm_mask
        << (quint16)h3.toUInt(&ok,16);

    // write global (first block)
    stream << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(5))).toUInt(&ok,2));
    stream << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(4))).toUInt(&ok,2));
    stream << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(3))).toUInt(&ok,2));

    // write channels
    for(int i = 63; i >=0; i--)
    {
        QString first8bits = QString::fromStdString(channel_vec.at(i)).mid(8,8);
        QString second16bits = QString::fromStdString(channel_vec.at(i)).mid(16,16);

        stream << (quint8)(first8bits).toUInt(&ok,2);
        stream << (quint16)(second16bits).toUInt(&ok,2);
    } // i

    // write global (second block)
    stream << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(2))).toUInt(&ok,2));
    stream << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(1))).toUInt(&ok,2));
    stream << (quint32)(vts::spi::reverseString(QString::fromStdString(global_vec.at(0))).toUInt(&ok,2));

}

} // namespace vts
