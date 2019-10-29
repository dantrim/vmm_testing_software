#ifndef VTS_SERVER_H
#define VTS_SERVER_H

//
// QT
//
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>

namespace vts
{

    class VTSServer : QObject
    {
        Q_OBJECT

        public :
            explicit VTSServer(QWidget* parent = 0);
            ~VTSServer();

            std::string trim(const std::string& str,
                    const std::string& whitespace = " \t");
            bool isASCII(const std::string& s);
            std::string QByteArrayToValidString(QByteArray& json_byte_array);

        private :

            QTcpServer m_server;
            QList<QTcpSocket*> m_sockets;
            bool m_validate_json_string;

        public slots :
            void onNewConnection();
            void onSocketStateChanged(QAbstractSocket::SocketState socketState);
            void onReadyRead();

    }; // class VTSServer

} // namespace vts

#endif
