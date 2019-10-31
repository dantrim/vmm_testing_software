#!/usr/bin/env python3
from __future__ import print_function

# Qt
from PySide2 import QtCore, QtNetwork

# misc
import json

class VTSCommunicator :
    def __init__(self) :
        self.cmd_id = -1

    def send_message(self, socket = None, message_data = {}, expect_reply = False, cmd_type = "TEST") :

        """
        Send data with the TCP socket \'socket\', assumed to already
        be connected
        """

        cmd_id = (self.cmd_id + 1)
        message = {
            "ID" : cmd_id
            ,"TYPE" : cmd_type
            ,"EXPECTS_REPLY" : expect_reply
            ,"DATA" : message_data
        }
        data = json.dumps(message, ensure_ascii = True)
        data = bytearray(data, encoding = "utf-8")
        data = QtCore.QByteArray(data)
        socket.write(data)

        reply = {}
        if not expect_reply :
            _ = socket.waitForReadyRead(1)
            _ = socket.readAll()
            return reply

        if socket.waitForReadyRead() : #5000) :
            reply = str(socket.readAll(), "utf-8")
            reply = json.loads(reply)

            cmd_id_recvd = reply["CMD_ID"]
            cmd_id_sent = cmd_id
            if cmd_id_recvd != cmd_id_sent :
                print("WARNING Received reply for command id not in sync with sent command (CMD_ID_RECVD={}, CMD_ID_SENT={})".format(cmd_id_recvd, cmd_id_sent))
        self.cmd_id = cmd_id
        return reply
