#!/usr/bin/env python3
from __future__ import print_function

# Qt
from PySide2 import QtCore, QtNetwork
from PySide2.QtCore import Signal, Slot
import threading

# system
import sys, os, subprocess, psutil
from pathlib import Path
import time
import json
import glob
import socket

# vts
from vts_utils import vts_comm
from peripherals import device_capture

# constants
STATUS_MONITOR_IP="127.0.0.1"
STATUS_MONITOR_PORT=1236

class VTSClient(QtCore.QObject) :

    def __init__(self, parent = None, config = None, config_file = "") :
        super(VTSClient, self).__init__(parent)

        self.config = config
        self.config_file = config_file
        self.comms = vts_comm.VTSCommunicator()
        self.server_process = None
        self.keep_monitor = False
        self.status_thread = None

    ##
    ## SIGNALS
    ##
    signal_vmm_sn_updated = Signal(str)
    signal_server_status_updated = Signal(str)
    signal_test_status_updated = Signal(str)
    signal_start_of_test = Signal(str, int)
    signal_end_of_test = Signal(str, str, str, str)

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

    def monitor(self, stop_func) :

        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(0.2)
        addr = (STATUS_MONITOR_IP,STATUS_MONITOR_PORT)
        sock.bind(addr)
        while True :
            if stop_func() :
                break
            try :
                data = str(sock.recv(1024), "utf-8")
                data = json.loads(data)
                monitor_type = data["TYPE"]
                if monitor_type == "TEST_STATUS" :
                    self.signal_test_status_updated.emit(data["DATA"])
                elif monitor_type == "START_OF_TEST" :
                    #print("SOT {}".format(data))
                    msg = data["DATA"]
                    current_test = msg["TEST_NAME"]
                    current_test_idx = msg["TEST_IDX"]
                    self.signal_start_of_test.emit(current_test, int(current_test_idx))
                elif monitor_type == "END_OF_TEST" :
                    print("EOT {}".format(data))
                    msg = data["DATA"]
                    test_completion_status = msg["TEST_COMPLETION"]
                    n_tests_run = msg["N_TESTS_TOUCHED"]
                    n_tests_exp = msg["N_TESTS_LOADED"]
                    last_test_run = msg["LAST_TEST_TOUCHED"]
                    self.signal_end_of_test.emit(test_completion_status, n_tests_run,  n_tests_exp, last_test_run)
            except socket.timeout :
                continue
            #self.signal_test_status_updated.emit(data)

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
        found_server = self.ping_server(quiet = False)
        if vts_found or found_server :
            print("VTS server already appears to be running at process PID={}".format(vts_pid))
            return

        path = Path(self.config["binary_location"])# / self.config["binary_name"])
        executable = path / self.config["binary_name"]
        if not executable.exists() :
            raise Exception("Executable path (={}) does not exist".format(str(executable)))
        
        pid = subprocess.Popen([str(executable), '--config', str(os.path.abspath(self.config_file))])
        print("VTS server starting (process={}, pid={})".format(self.config["binary_name"], pid.pid))
        self.server_pid = int(pid.pid)
        self.server_process = pid

        # allow for some time to pass to ensure that everything is running before we return
        if wait > 0 :
            time.sleep(wait)

        found_server = self.ping_server(quiet = True)
        
        self.keep_monitor = False
        self.status_thread = threading.Thread(target = self.monitor, args = (lambda : self.keep_monitor,))
        self.status_thread.start()

    def kill_server(self, wait = 1) :

        ##
        ## First, stop the monitoring thread if it is running
        ##
        if self.status_thread is not None :
            self.keep_monitor = True
            self.status_thread.join()

        ##
        ## check if any server is even running
        ##
        vts_found, vts_pid = self.check_for_vts(by_name = True)
        if not vts_found :
            print("VTS server does not appear to be running")
            return

        address = (self.config["server_ip"], int(self.config["server_port"]))
        message = {
            "CMD" : "EXIT"
        }
        _ = self.comms.send_message_socket(address = address,
                message_data = message,
                expect_reply = False,
                cmd_type = "SERVER"
        )

        time.sleep(wait)
        found_server = self.ping_server(quiet = True)
        return

    def ping_server(self, close_after = True, quiet = False) :

        ###[START]
        found_server = False
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s :
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
            address = (self.config["server_ip"], int(self.config["server_port"]))
            result = s.connect_ex(address)
            if result :
                found_server = False
            else :
                s.shutdown(socket.SHUT_WR)
                found_server = True
        ping_status_str = { True : "Alive", False : "Dead" }[found_server]
        self.signal_server_status_updated.emit(str(ping_status_str))
        if not quiet :
            print("VTS Server alive? {}".format(found_server))
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

    def frontend_cmd(self, cmd = "", expect_reply = True, wait = 5000) :

        data = {
            "CMD" : cmd
        }
        address = (self.config["server_ip"], int(self.config["server_port"]))
        reply = self.comms.send_message_socket(address = address,
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

    def test_cmd(self, cmd = "", test_data = {}, expect_reply = True, wait = 5000) :
        data = {
            "CMD" : cmd,
            "TEST_DATA" : test_data
        }
        address = (self.config["server_ip"], int(self.config["server_port"]))
        #if cmd == "STOP" :
        #    address = (self.config["server_ip"], 1237)
        #    self.comms.send_message_udp(address = address)
        #    reply = {}
        #else :
        reply, cmd_id = self.comms.send_message_socket(address = address,
                    message_data = data,
                    expect_reply = expect_reply,
                    cmd_type = "VMMTEST",
                    wait = wait
        )
        print("reply? {}".format(reply))
        return reply


    @Slot(str)
    def load_test(self, test_names = [], test_config_files = []) :

        print("test names = {}".format(test_names))
        print("VTS Client load_test: test names={}".format(test_names))
        print("VTS Client load_test: test cfg  ={}".format(test_config_files))

        # first acquire the current VMM serial ID
        camera = device_capture.PictureTaker()
        vmm_sn = camera.get_serial_number()
        if not vmm_sn :
            print("ERROR Could not acquire VMM serial number - Cannot load tests!")
            return
        test_data = {
            "VMM_SERIAL_ID" : vmm_sn,
            "TEST_CONFIG" : test_config_files
        }

        reply = self.test_cmd(cmd = "LOAD", test_data = test_data) #test_files)
        status = reply["STATUS"] == "OK"
        if not status :
            print("ERROR Failed to load tests, reply: {}".format(reply))
        return status

    def start_test(self) :
        self.test_cmd(cmd = "START", expect_reply = False)
        return True

    def stop_test(self) :
        self.test_cmd(cmd = "STOP")

    def capture_vmm_serial(self) :
        camera = device_capture.PictureTaker()
        vmm_sn = camera.get_serial_number()
        if not vmm_sn :
            print("ERROR Could not acquire VMM serial number")
        
        self.signal_vmm_sn_updated.emit(str(vmm_sn))
        return vmm_sn
