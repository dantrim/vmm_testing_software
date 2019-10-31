#!/usr/bin/env python3
from __future__ import print_function

# Qt
from PySide2 import QtCore, QtNetwork

# system
import sys, os, subprocess, psutil
from pathlib import Path
import time
import json

# vts
from vts_utils import vts_comm

class VTSClient(QtCore.QObject) :
    def __init__(self, parent = None, config = None) :
        super(VTSClient, self).__init__(parent)

        self.socket = QtNetwork.QTcpSocket(self)
        self.config = config
        self.comms = vts_comm.VTSCommunicator()

    def check_for_vts(self, by_name = False) :

        for proc in psutil.process_iter() :
            try :
                pinfo = proc.as_dict(attrs = ["pid", "name"])
                if not by_name :
                    return self.pid_exists(int(self.server_pid))
                elif by_name and pinfo["name"] == self.config["binary_name"] :
                    return True, int(pinfo["pid"])
            except :
                pass
        return False, -1
        self.server_pid = -1

    def pid_exists(self, pid_num) :

        for proc in psutil.process_iter() :
            try :
                pinfo = proc.as_dict(attrs = ["pid", "name"])
                if int(pinfo["pid"]) == int(pid_num) :
                    return True
            except :
                pass
        return False

    def start_server(self, wait = 1) :

        vts_found, vts_pid = self.check_for_vts(by_name = True)
        if vts_found :
            raise Exception("VTS server already appears to be running at process PID={}".format(vts_pid))

        path = Path(self.config["binary_location"])# / self.config["binary_name"])
        executable = path / self.config["binary_name"]
        if not executable.exists() :
            raise Exception("Executable path (={}) does not exist".format(str(executable)))
        
        pid = subprocess.Popen([str(executable), '&'])
        print("VTS server starting (process={}, pid={})".format(self.config["binary_name"], pid.pid))
        self.server_pid = int(pid.pid)

        # allow for some time to pass to ensure that everything is running before we return
        if wait > 0 :
            time.sleep(wait)

    def kill_server(self) :

        ##
        ## check if any server is even running
        ##
        vts_found, vts_pid = self.check_for_vts(by_name = True)
        if not vts_found :
            raise Exception("VTS server does not appear to be running")

        ##
        ## prepare the socket for a new message
        ##
        self.reset_socket()

        ##
        ## prepare the data to send
        ##
        message = {
            "CMD" : "EXIT"
        }

        attempts = 0
        while self.pid_exists(vts_pid) :

            if attempts > 5 :
                raise Exception("Failed to kill VTS server after 5 attempts") 

            _ = self.comms.send_message(socket = self.socket,
                        message_data = message,
                        expect_reply = False,
                        cmd_type = "SERVER")

            time.sleep(0.2)
            attempts += 1
        self.server_pid = -1

    def ping_server(self, close_after = True) :

        self.socket.abort()
        ip, port = self.config["server_ip"], self.config["server_port"]
        self.socket.connectToHost(ip, int(port))
        attempts = 0
        while True :
            if attempts > 5 :
                self.socket.close()
                return False
            if self.socket.waitForConnected(3000) :
                if close_after :
                    self.socket.close()
                return True
            attempts += 1

    def clean(self, by_name = False) :

        for proc in psutil.process_iter() :
            try :
                pinfo = proc.as_dict(attrs = ["pid", "name"])
                if pinfo["name"] == self.config["binary_name"] :
                    print("Terminating VTS process with (name,PID)=({},{})".format(pinfo["name"],pinfo["pid"]))
                    proc.terminate()
            except :
                pass

    def reset_socket(self) :
        
        self.socket.abort()
        ip, port = self.config["server_ip"], self.config["server_port"]
        self.socket.connectToHost(ip, int(port))
        if not self.ping_server(close_after = False) :
            raise Exception("Failed to reset TCP socket")

    def dummy_send(self) :

        self.reset_socket()

        ##
        ## prepare the data to send
        ##

        dummy_data = {
            "CMD_ID" : 1,
            "CMD" : "UP"
        }
        reply = self.comms.send_message(socket = self.socket,
                    message_data = dummy_data,
                    expect_reply = True
        )
        print("reply? {}".format(reply))
