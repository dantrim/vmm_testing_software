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
    def __init__(self, parent = None, config = None, config_file = "") :
        super(VTSClient, self).__init__(parent)

        self.socket = QtNetwork.QTcpSocket(self)
        self.config = config
        self.config_file = config_file
        self.comms = vts_comm.VTSCommunicator()
        self.server_process = None

    def check_for_vts(self, by_name = False) :

        for proc in psutil.process_iter() :
            try :
                pinfo = proc.as_dict(attrs = ["pid", "name", "status"])
                is_zombie = pinfo["status"].lower() == "zombie"
                is_actually_running = not is_zombie
                if not by_name :
                    return self.pid_exists(int(self.server_pid)) and is_actually_running
                elif by_name and pinfo["name"] == self.config["binary_name"] :
                    return is_actually_running, int(pinfo["pid"])
            except :
                pass
        self.server_pid = -1
        return False, -1

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
        
        pid = subprocess.Popen([str(executable), '--config', str(os.path.abspath(self.config_file))])
        #pid = subprocess.call([str(executable), '--config', str(os.path.abspath(self.config_file))])
        print("VTS server starting (process={}, pid={})".format(self.config["binary_name"], pid.pid))
        self.server_pid = int(pid.pid)
        self.server_process = pid

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
        while attempts == 0 : #self.pid_exists(vts_pid) :

            if attempts > 5 :
                raise Exception("Failed to kill VTS server after 5 attempts") 

            _ = self.comms.send_message(socket = self.socket,
                        message_data = message,
                        expect_reply = False,
                        cmd_type = "SERVER")
            self.socket.close()

            time.sleep(0.2)
            attempts += 1

        #self.clean(by_name = True)
        self.server_pid = -1

    def ping_server(self, close_after = True, quiet = False) :

        self.socket.abort()
        ip, port = self.config["server_ip"], self.config["server_port"]
        self.socket.connectToHost(ip, int(port))
        attempts = 0
        found_server = False
        while True :
            if attempts > 5 :
                #self.socket.close()
                found_server = False
                break
            if self.socket.waitForConnected(3000) :
                #if close_after :
                #    self.socket.close()
                found_server = True
                break
            attempts += 1

        # clear and close
        if not found_server or close_after :
            _ = self.socket.waitForReadyRead(1)
            _ = self.socket.readAll()
            self.socket.close()

        if not quiet :
            print("ping? {}".format(found_server))
        return found_server

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
        if not self.ping_server(close_after = False, quiet = True) :
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

    def frontend_cmd(self, cmd = "", expect_reply = True, wait = 5000) :

        self.reset_socket()
        data = {
            "CMD" : cmd
        }
        reply = self.comms.send_message(socket = self.socket,
                    message_data = data,
                    expect_reply = True,
                    cmd_type = "FRONTEND",
                    wait = wait
        )
        print("reply? {}".format(reply))

    def board_on(self) :
        self.frontend_cmd(cmd = "POWERON")

    def board_off(self) :
        self.frontend_cmd(cmd = "POWEROFF")

    def ping_fpga(self) :
        self.frontend_cmd(cmd = "PINGFPGA")

    def configure_fpga(self) :
        self.frontend_cmd(cmd = "CONFIGUREFPGA")

    def acq_on(self) :
        self.frontend_cmd(cmd = "ACQON")

    def acq_off(self) :
        self.frontend_cmd(cmd = "ACQOFF")

    def configure_vmm(self) :
        self.frontend_cmd(cmd = "CONFIGUREVMM")

    def reset_vmm(self) :
        self.frontend_cmd(cmd = "RESETVMM")
