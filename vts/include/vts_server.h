#ifndef VTS_SERVER_H
#define VTS_SERVER_H

//
// QT
//
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>

//
// logging
//
namespace spdlog {
    class logger;
}

//
// std
//
#include <memory>

namespace vts
{

    class VTSServer : QObject
    {
        Q_OBJECT

        public :
            explicit VTSServer(QWidget* parent = 0);
            ~VTSServer();
            bool start();
            bool stop();
            bool up() { return m_is_running; }

            std::string trim(const std::string& str,
                    const std::string& whitespace = " \t");
            bool isASCII(const std::string& s);
            std::string QByteArrayToValidString(QByteArray& json_byte_array);

        private :

            QTcpServer m_server;
            QList<QTcpSocket*> m_sockets;
            bool m_is_running;
            bool m_validate_json_string;
            std::shared_ptr<spdlog::logger> log;

        public slots :
            void onNewConnection();
            void onSocketStateChanged(QAbstractSocket::SocketState socketState);
            void onReadyRead();

        signals :
            void closeServer();

    }; // class VTSServer

} // namespace vts

#endif
