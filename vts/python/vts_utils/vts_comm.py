#!/usr/bin/env python3
from __future__ import print_function

# Qt
from PySide2 import QtCore, QtNetwork

# socket
import socket

# misc
import json
import time

class VTSCommunicator :
    def __init__(self) :
        self.cmd_id = -1

    def close_socket(socket, how) :
        try :
            socket.shutdown(how)
            socket.close()
        except :
            return

    def send_message_socket(self, address = (), message_data = {}, expect_reply = False, cmd_type = "TEST", wait = 1) :

        self.cmd_id = self.cmd_id + 1
        reply = {}
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s :
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
            s.settimeout(2) # set the timeout for 1 second of no activity
            result = s.connect_ex(address)
            connected_ok = not result
            if not connected_ok :
                print("ERROR Could not connect to VTS for command id {}".format(self.cmd_id))
                return
            message = {
                "ID" : self.cmd_id
                ,"TYPE" : cmd_type
                ,"EXPECTS_REPLY" : expect_reply
                ,"DATA" : message_data
            }
            data = json.dumps(message, ensure_ascii = True)
            data = bytearray(data, encoding = "utf-8")
            s.sendall(data)
            if expect_reply :
                try :
                    reply = str(s.recv(1024), "utf-8")
                    is_empty = reply == ""
                    reply = json.loads(reply)
                    cmd_id_rcvd = reply["ID"]
                    cmd_id_sent = self.cmd_id
                    if cmd_id_rcvd != cmd_id_sent :
                        print("WARNING Received reply for command id not in sync with sent command (CMD_ID_RECVD={}, CMD_ID_SENT={})".format(cmd_id_rcvd, cmd_id_sent))
                        
                    reply = reply["DATA"]
                except socket.timeout :
                    print("WARNING Did not receive reply in time [TIMEOUT]")
                    reply = {}
        return reply, self.cmd_id

    def send_message_udp(self, address = (), message_data = {}) :

            self.cmd_id = self.cmd_id + 1
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s :
                data = "dummy"
                data = bytearray(data, encoding = "utf-8")
                s.sendto(data, address)
        

    def send_message(self, socket = None, message_data = {}, expect_reply = False, cmd_type = "TEST", wait = 1) :

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
            #_ = socket.waitForReadyRead(wait)
            _ = socket.readAll()
            return reply

        if socket.waitForReadyRead(wait) : #5000) :
            reply = str(socket.readAll(), "utf-8")
            reply = json.loads(reply)

            cmd_id_rcvd = reply["ID"]
            cmd_id_sent = cmd_id
            if cmd_id_rcvd != cmd_id_sent :
                print("WARNING Received reply for command id not in sync with sent command (CMD_ID_RECVD={}, CMD_ID_SENT={})".format(cmd_id_rcvd, cmd_id_sent))
            reply_message = reply["DATA"]
        self.cmd_id = cmd_id


        return reply, self.cmd_id
