#!/bin/env python

# Qt
from PySide2 import QtCore, QtNetwork
import time
import os

CAMERA_IP = "127.0.0.1"
CAMERA_SEND_PORT = 2005
CAMERA_RECV_PORT = 2006

import threading

class CameraEmulator(QtCore.QObject) :
    def __init__(self, parent = None) :
        super(CameraEmulator, self).__init__(parent)

        self.send_server = QtNetwork.QTcpServer(self)
        self.socket = QtNetwork.QTcpSocket(self)

        self.send_sockets = []
        print("Starting Camera")

    def reset_socket(self) :
        self.socket.abort()
        self.socket.connectToHost(CAMERA_IP, CAMERA_RECV_PORT)

    def reset(self) :
        self.send_sockets = []

    def start(self) :

        self.send_server.listen(QtNetwork.QHostAddress(CAMERA_IP), int(CAMERA_SEND_PORT))
        self.send_server.newConnection.connect(self.onNewConnectionSend)

    def onNewConnectionSend(self) :

        connected_client = self.send_server.nextPendingConnection()
        addr, port = connected_client.peerAddress().toString(), connected_client.peerPort()

        connected_client.readyRead.connect(self.onReadyReadSend)
        self.send_sockets.append(connected_client)

    def onReadyReadSend(self) :

        if len(self.send_sockets) > 1 :
            tmp = self.send_sockets[-1]
            self.send_sockets = [tmp]

        data = str(self.send_sockets[0].read(3), "utf-8")

        if data == "TRG" :
            self.reset_socket()

            serial = "|-51367-|"
            serial = bytearray(serial, encoding = "utf-8")
            serial = QtCore.QByteArray(serial)
            n_bytes = self.socket.write(serial)
        else :
            print("Warning: Camera does not know how to respond to given command \"{}\"".format(data))
class CameraComm(QtCore.QObject) :

    def __init__(self, parent = None) :
        super(CameraComm, self).__init__(parent)

        self.recv_server = QtNetwork.QTcpServer(self)
        self.socket = QtNetwork.QTcpSocket(self)

        self.recv_sockets = []

        self._current_serial = "NULL"

    def start(self) :

        self.recv_server.listen(QtNetwork.QHostAddress(CAMERA_IP), int(CAMERA_RECV_PORT))
        self.recv_server.newConnection.connect(self.onNewConnectionRecv)

    def onNewConnectionRecv(self) :

        connected_client = self.recv_server.nextPendingConnection()
        addr, port = connected_client.peerAddress().toString(), connected_client.peerPort()

        connected_client.readyRead.connect(self.onReadyReadRecv)
        self.recv_sockets.append(connected_client)

    def onReadyReadRecv(self) :

        if len(self.recv_sockets) > 1 :
            tmp = self.recv_sockets[-1]
            self.recv_sockets = [tmp]

        data = str(self.recv_sockets[0].readAll(), "utf-8")
        serial = data.replace("|-","").replace("-|","")
        self._current_serial = serial

    def reset_socket(self) :
        self.socket.abort()
        self.socket.connectToHost(CAMERA_IP, CAMERA_SEND_PORT)

    def connect_to_camera(self) :
        self.reset_socket()
        return True

    def request_serial(self) :

        request = "TRG"
        request = bytearray(request, encoding = "utf-8")
        request = QtCore.QByteArray(request)
        n_bytes = self.socket.write(request)

    def current_serial(self) :
        return self._current_serial
