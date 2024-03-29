#!/usr/bin/env python3

import sys, os
from pathlib import Path
import argparse
import json
import time


# VTS
from interface import vts_mainwindow
from vts_utils import vts_helpers, vts_client
import socket


# Qt
from PySide2 import QtCore, QtGui, QtWidgets, QtNetwork
from PySide2.QtCore import Slot, Signal

#ui->monitoring_enable_button->setStyleSheet("QPushButton {background-color: rgb(15, 147, 255);}");

VTS_GREEN = "(37, 229, 94)"
VTS_WHITE = "(255, 255, 255)"
VTS_RED = "(233, 40, 56)"
VTS_BLUE = "(84, 163, 252)"
VTS_GREY = "(189, 189, 189)"
VTS_YELLOW = "(244, 255, 122)"

class VTSWindow(QtWidgets.QMainWindow) :
    def __init__(self, client = None) :
        super(VTSWindow, self).__init__()
        self.vts = client
        self.ui = vts_mainwindow.Ui_MainWindow()
        self.setFixedSize(871,372)
        self.ui.setupUi(self)
        self.setup_vts_connections(ui = self.ui)
        self.setup_defaults(ui = self.ui)

        #self.ui.button_dummy.setVisible(False)

    def set_background(self, obj = None, obj_type_str = "", color = VTS_WHITE, reset = False) :

        if reset :
            color = VTS_WHITE
        bkg = "rgb{};".format(color)
        bkg = "%s {background-color: %s}" % (obj_type_str, bkg)
        obj.setStyleSheet(bkg)

    ##
    ## VTS SIGNALS
    ##
    signal_vts_config_updated = Signal()
    signal_vmm_status = Signal(str)

    ##
    ## VTS SLOTS
    ##

    @Slot(str)
    def vmm_sn_updated(self, value) :
        bkg_color = "rgb{};".format(VTS_GREY)
        text = "none"
        if value == "" :
            bkg_color = "rgb{};".format(VTS_RED)
            text = "none"
        else :
            bkg_color = "rgb{};".format(VTS_GREEN)
            text = "VMM# {}".format(str(value))
        bkg_color = "QLabel {background-color: %s}" % bkg_color
        self.ui.label_vts_vmm_sn.setStyleSheet(bkg_color)
        self.ui.label_vts_vmm_sn.setText(text)

    @Slot(str)
    def server_status_updated(self, value) :
        if value not in ["Alive", "Dead"] :
            bkg_color = "rgb{};".format(VTS_GREY)
        else :
            bkg_color = { "Alive" : VTS_GREEN, "Dead" : VTS_RED }[value]
            bkg_color = "rgb{};".format(bkg_color)
        bkg_color = "QLabel {background-color: %s}" % bkg_color
        self.ui.label_vts_server_status.setStyleSheet(bkg_color)
        text = "SERVER " + str(value).upper()
        self.ui.label_vts_server_status.setText(text)

    def load_test_config_from_dir(self, test_config_dir = "", send_to_server = True, vmm_sn = "") :

        self.ui.progressBar_current_test_progress.setValue(0)

        test_config_dir = str(test_config_dir)
        p_test_config_dir = Path(test_config_dir)
        exists_and_is_dir = p_test_config_dir.exists() and p_test_config_dir.is_dir()
        if not exists_and_is_dir :
            self.set_background(obj = self.ui.lineEdit_test_dir
                                ,obj_type_str = "QLineEdit"
                                ,color = VTS_RED
            )
            self.clear_tests()
            return
        self.ui.lineEdit_test_dir.setText(str(p_test_config_dir))

        # don't let the user do this if the server isn't alive
        server_running = self.vts.ping_server(quiet = True)
        if not server_running :
            self.set_background(obj = self.ui.lineEdit_test_dir
                    ,obj_type_str = "QLineEdit"
                    ,reset = True
            )
            return

        test_dict = vts_helpers.tests_from_test_dir(p_test_config_dir)

        if len(test_dict) >= 0 :
            reset = len(test_dict) == 0
            self.set_background(obj = self.ui.lineEdit_test_dir
                                ,obj_type_str = "QLineEdit"
                                ,color = VTS_GREEN
                                ,reset = reset
            )

        n_tests = len(test_dict)
        for test_name, test_config_file in test_dict.items() :
            self.ui.listWidget_loaded_tests.addItem(test_name)

        self.ui.label_current_test.setText("TESTS LOADED")
        self.set_background(obj = self.ui.label_current_test, obj_type_str = "QLabel", color = VTS_GREY)

        # assumes that VTS server is running
        if server_running and send_to_server :
            self.send_tests_to_vts(vmm_sn = vmm_sn)
            self.ui.label_current_test.setText("TESTS LOADED")
            self.set_background(obj = self.ui.label_current_test, obj_type_str = "QLabel", color = VTS_BLUE)

    @Slot()
    def load_test_config(self) :

        # don't continue unless we have acquired VMM serial number
        vmm_sn_text = self.ui.label_vts_vmm_sn.text()
        if vmm_sn_text.lower() == "none" :
            print("ERROR No VMM serial number acquired, cannot load tests")
            return

        vmm_sn = self.ui.lineEdit_manual_vmm_sn.text()

        self.clear_tests()
        test_config_dir = self.ui.lineEdit_test_dir.text()
        self.load_test_config_from_dir(test_config_dir = str(test_config_dir), vmm_sn = vmm_sn)


    def get_loaded_tests(self) :

        test_names = []
        test_configs = []
        current_test_dir = self.ui.lineEdit_test_dir.text()


        n_items = self.ui.listWidget_loaded_tests.count()
        self.ui.label_test_count.setText("0/{}".format(n_items))
        for itest in range(n_items) :
            item = self.ui.listWidget_loaded_tests.item(itest)
            item_name = item.text()
            item_config = vts_helpers.config_file_for_test(item_name, Path(current_test_dir))
            if item_config is None :
                print("WARNING No config file found for test \"{}\"".format(item_name))
                continue
            test_names.append(item_name)
            test_configs.append(str(item_config))
        return test_names, test_configs

    @Slot()
    def clear_tests(self) :
        self.ui.listWidget_loaded_tests.clear()
        self.new_test_started("", 0)

    def send_tests_to_vts(self, vmm_sn = "") :

        test_names, test_config_files = self.get_loaded_tests()
        status = self.vts.load_test(test_names = test_names, test_config_files = test_config_files, vmm_sn = vmm_sn)
        if not status :
            self.set_background(obj = self.ui.lineEdit_test_dir
                                ,obj_type_str = "QLineEdit"
                                ,color = VTS_RED
                                ,reset = False
            )
            self.clear_tests()
        else :
            self.set_background(obj = self.ui.lineEdit_test_dir
                                ,obj_type_str = "QLineEdit"
                                ,color = VTS_GREEN
                                ,reset = False
            )
        return

    @Slot()
    def start_server(self) :
        status = self.vts.start_server(wait = 0.2)

    @Slot()
    def toggle_server(self) :
        is_on = self.ui.button_start_vts_server.isChecked()
        if is_on :
            self.ui.button_start_vts_server.setText("Shutdown")
            status = self.vts.start_server(wait = 0.2)
            self.ui.button_tests_start.setEnabled(False)
            self.ui.button_tests_stop.setEnabled(False)
            self.ui.button_tests_load.setEnabled(True)
            self.ui.button_ping_fpga.setEnabled(True)
            self.ui.button_ping_fpga2.setEnabled(True)
        else :
            self.vts.kill_server(wait = 0.1)
            self.ui.button_start_vts_server.setText("Start")
            self.ui.button_tests_start.setEnabled(False)
            self.ui.button_tests_stop.setEnabled(False)
            self.ui.button_tests_load.setEnabled(False)
            self.ui.button_ping_fpga.setEnabled(False)
            self.ui.button_ping_fpga2.setEnabled(False)
            self.reset_status()
        self.ui.button_ping_vts_server.click()

    def reset_status(self) :
        labels = [self.ui.label_fpga_status,
                    self.ui.label_vmm_status,
                    self.ui.label_vts_vmm_sn,
                    self.ui.label_current_test,]
        for label in labels :
            label.setText("none")
            self.set_background(obj = label, obj_type_str = "QLabel", color = VTS_GREY)
        self.ui.progressBar_current_test_progress.setValue(0)
        self.set_background(obj = self.ui.label_test_status, obj_type_str = "QLabel", color = VTS_GREY)
        self.ui.label_test_status.setText("none")

        self.ui.button_tests_load.setEnabled(False)
        

    @Slot()
    def kill_server(self) :
        self.vts.kill_server(wait = 0.1)

    @Slot()
    def start_tests(self) :

        # don't let us start if there is no VMM sn
        vmm_sn_text = self.ui.label_vts_vmm_sn.text()
        if vmm_sn_text.lower() == "none" :
            print("ERROR No VMM serial number acquired, cannot start tests")
            return

        status = self.vts.start_test()
        if status :
            self.ui.button_tests_stop.setEnabled(True)
            self.ui.button_tests_start.setEnabled(False)
    
            # update ui
            self.ui.label_test_status.setText("TESTING")
            self.set_background(obj = self.ui.label_test_status,
                                    obj_type_str = "QLabel",
                                    color = VTS_BLUE)

    @Slot()
    def stop_tests(self) :
        self.vts.stop_test()
        self.ui.button_tests_stop.setEnabled(False)
        self.ui.button_tests_start.setEnabled(True)

        self.ui.label_test_status.setText("TESTING STOPPED")
        self.set_background(obj = self.ui.label_test_status,
                                    obj_type_str = "QLabel",
                                    color = VTS_GREY)
        

    def test_status_updated(self, status) :

        percentage = 100. * float(status)
        self.ui.progressBar_current_test_progress.setValue(int(percentage))

    @Slot(str,int)
    def new_test_started(self, test_name, test_idx) :
        n_tests = self.ui.listWidget_loaded_tests.count()
        test_idx = test_idx+1
        if n_tests == 0 :
            test_idx = 0
        test_str = "{}/{}".format(str(test_idx), str(n_tests))
        self.ui.label_test_count.setText(test_str)

        if test_name != "" :
            self.ui.label_current_test.setText(test_name)
            self.set_background(obj = self.ui.label_current_test,
                                    obj_type_str = "QLabel",
                                    color = VTS_GREEN)

    @Slot(str)
    def test_ended(self, final_test_result = "") :

        self.ui.button_tests_stop.click()

        color = VTS_GREEN
        color = {
            "SUCCESS" : VTS_GREEN
            ,"PASS" : VTS_YELLOW
            ,"FAIL" : VTS_RED
            ,"" : VTS_GREY
            ,"NONE" : VTS_GREY
            ,"INCOMPLETE" : VTS_GREY
        }[final_test_result]
        self.ui.label_test_status.setText(final_test_result)
        self.set_background(obj = self.ui.label_test_status,
                                obj_type_str = "QLabel",
                                color = color)

    @Slot()
    def ping_visor_camera(self) :
        is_alive = True

        camera_ip = self.ui.lineEdit_camera_ip.text()
        try :
            socket.inet_aton(camera_ip)
        except socket.error :
            print("ERROR VISOR IPv4 (={}) is invalid".format(camera_ip))
            is_alive = False
            self.set_background(obj = self.ui.lineEdit_camera_ip, obj_type_str = "QLineEdit", color = VTS_RED)

        if is_alive :
            is_alive = self.vts.ping_visor_camera(camera_ip = camera_ip)
            color = { True : VTS_GREEN,
                    False : VTS_RED }[is_alive]
            self.set_background(obj = self.ui.lineEdit_camera_ip, obj_type_str = "QLineEdit", color = color)

    @Slot()
    def capture_vmm_serial(self) :

        vmm_sn = ""
        # get and check the camera IP address
        camera_ip = self.ui.lineEdit_camera_ip.text()

        do_manual =  self.ui.button_do_manual_vmm_sn.isChecked()
        if do_manual :
            vmm_sn_text = self.ui.lineEdit_manual_vmm_sn.text()
            if vmm_sn_text.isdigit() :
                vmm_sn = self.vts.capture_vmm_serial(manual = vmm_sn_text, camera_ip = camera_ip)
            else :
                print("ERROR Unable to acquire manual VMM serial number as it is not a number: {}".format(vmm_sn_text))
                self.vmm_sn_updated("")
                vmm_sn = ""
        else :

            try :
                socket.inet_aton(camera_ip)
                vmm_sn = self.vts.capture_vmm_serial(manual = "", camera_ip = camera_ip)
            except socket.error :
                print("ERROR VISOR IPv4 (={}) is invalid".format(camera_ip))
                vmm_sn = ""

        if vmm_sn == "" :
            self.ui.button_tests_start.setEnabled(False)
            self.ui.button_tests_stop.setEnabled(False)
        else :
            self.ui.button_tests_start.setEnabled(True)
            self.ui.button_tests_stop.setEnabled(False)

        self.ui.lineEdit_manual_vmm_sn.setText(vmm_sn)
        return vmm_sn

    @Slot()
    def ping_fpga(self) :
        server_running = self.ui.button_start_vts_server.isChecked()
        reply_status = False
        if server_running :
            reply_status = self.vts.ping_fpga()
        text_str, color = { True : ["FPGA ALIVE",VTS_GREEN]
                    ,False : ["FPGA DEAD",VTS_RED] }[reply_status]
        if not server_running :
            text_str = "none"
            color = VTS_GREY
        self.ui.label_fpga_status.setText(text_str)
        self.set_background(obj = self.ui.label_fpga_status, obj_type_str = "QLabel", color = color)

        # for now just assume that VMM is OK
        if reply_status :
            self.signal_vmm_status.emit("ON")
        else :
            if server_running :
                self.signal_vmm_status.emit("OFF")
            else :
                self.signal_vmm_status.emit("NONE")

    @Slot(str)
    def update_vmm_status(self, status_str) :
        text_str, color = { "ON" : ["VMM ON",VTS_GREEN]
                    ,"OFF" : ["VMM OFF",VTS_RED]
                    ,"NONE" : ["none",VTS_GREY] } [status_str]
        self.ui.label_vmm_status.setText(text_str)
        self.set_background(obj = self.ui.label_vmm_status, obj_type_str = "QLabel", color = color)

    ##
    ## VTS CONNECTIONS
    ##

    def setup_vts_connections(self, ui = None) :

        ##
        ## VTS CONTROL
        ##
        ui.button_start_vts_server.clicked.connect(self.toggle_server)
        #ui.button_start_vts_server.clicked.connect(self.start_server)
        #ui.button_shutdown_vts_server.clicked.connect(self.kill_server)
        ui.button_ping_vts_server.clicked.connect(self.vts.ping_server)

        self.vts.signal_vmm_sn_updated.connect(self.vmm_sn_updated)
        self.vts.signal_server_status_updated.connect(self.server_status_updated)
        self.signal_vmm_status.connect(self.update_vmm_status)

        ##
        ## DEVICE CONTROL
        ##
        ui.button_acquire_vmm_serial.clicked.connect(self.capture_vmm_serial)
        ui.button_ping_camera.clicked.connect(self.ping_visor_camera)

        ##
        ## TESTS
        ##
        ui.button_tests_load.clicked.connect(self.load_test_config)
        ui.button_tests_start.clicked.connect(self.start_tests)
        ui.button_tests_stop.clicked.connect(self.stop_tests)

        ##
        ## FPGA
        ##
        ui.button_fpga_configure.clicked.connect(self.vts.configure_fpga)
        ui.button_ping_fpga.clicked.connect(self.ping_fpga)
        ui.button_ping_fpga2.clicked.connect(self.ping_fpga)

        ##
        ## VMM
        ##
        ui.button_vmm_configure.clicked.connect(self.vts.configure_vmm)
        ui.button_vmm_reset.clicked.connect(self.vts.reset_vmm)

        ##
        ## DAQ
        ##
        ui.button_acq_on.clicked.connect(self.vts.acq_on)
        ui.button_acq_off.clicked.connect(self.vts.acq_off)

        ##
        ## VTS CLIENT
        ##
        self.vts.signal_test_status_updated.connect(self.test_status_updated)
        self.vts.signal_start_of_test.connect(self.new_test_started)
        self.vts.signal_end_of_test.connect(self.test_ended)

    def setup_defaults(self, ui = None) :
        self.ui.progressBar_current_test_progress.setValue(0)

        ##
        ## VTS CONTROL
        ##

        # default vts config
        default_vts_config = vts_helpers.default_vts_config()
        ui.lineEdit_vts_config_file.setText(str(default_vts_config))

        with open(self.vts.config_file, "r") as cfg_file :
            vts_cfg = json.load(cfg_file)["vts_config"]

        ##
        ## FPGA
        ##
        frontend_cfg = vts_cfg["frontend"]
        board_ip = frontend_cfg["board_ip"]
        ui.lineEdit_fpga_ip.setText(board_ip)
        self.ui.button_ping_fpga.setEnabled(False)
        self.ui.button_ping_fpga2.setEnabled(False)

        ##
        ## peripherals
        ##
        if "peripherals" in vts_cfg :
            peripherals_cfg = vts_cfg["peripherals"]

        ## visor camera

            if "visor_camera" in peripherals_cfg :
                camera_cfg = peripherals_cfg["visor_camera"]
                camera_ip = camera_cfg["ip_address"]
                ui.lineEdit_camera_ip.setText(camera_ip)

        ##
        ## TESTING
        ##

        # loading
        self.ui.button_tests_load.setEnabled(False)

        # output location
        output_cfg = vts_cfg["test_output"]
        output_dir = output_cfg["output_directory"]
        self.ui.lineEdit_test_output_location.setText(output_dir)

        # tests
        ui.lineEdit_test_dir.clear()
        ui.listWidget_loaded_tests.clear()
        default_test_dir = vts_helpers.default_tests_dir()
        self.load_test_config_from_dir(str(default_test_dir), send_to_server = False)

def args_ok(args) :

    ok = True
    p_config_file = Path(args.config)
    if not p_config_file.exists() :
        print("ERROR Cannot find specified VTS configuration file: {}".format(args.config))
        ok = False
    return ok

def handle_server_command_and_exit(args, vts_client) :

    cmd_type = args.server.lower()
    if cmd_type == "ping" :
        vts_client.ping_server(quiet = False)
    elif cmd_type == "start" :
        vts_client.start_server()
    elif cmd_type == "stop" :
        vts_client.kill_server()
    elif cmd_type == "clean" :
        vts_client.clean()
    else :
        print("ERROR Unknown server command \"{}\" provided".format(cmd_type))

def main() :

    ##
    ## CLI
    ##
    parser = argparse.ArgumentParser(description = "VTS: VMM Testing Software")
    parser.add_argument("-c", "--config", default = "../config/vts_default.json",
            help = "VTS configuration"
    )
    parser.add_argument("--gui", action = "store_true", default = False,
            help = "Launch VTS GUI"
    )
    parser.add_argument("-s", "--server",
            choices = ["START", "STOP", "PING", "CLEAN"],
            help = "VTS server commands"
    )

    args = parser.parse_args()
    if not args_ok(args) :
        print("Unable to begin")
        sys.exit(1)

    ##
    ## load VTS configuration
    ##
    p = Path(args.config)
    with open(p,"r") as config_file :
        config_data = json.load(config_file)
        if "vts_config" not in config_data :
            print("ERROR \"vts_config\" top-level is not found in provided config file: {}".format(str(p)))
            sys.exit(1)
        config_data = config_data["vts_config"]
    vts_server_config, logging_config = config_data["vts_server"], config_data["logging"]

    ##
    ## instantiate VTS client
    ##
    client = vts_client.VTSClient(config = vts_server_config, config_file = args.config)

    ##
    ## process user directives
    ##
    if args.server :
        handle_server_command_and_exit(args, client)
        sys.exit(0)

    if args.gui :
        app = QtWidgets.QApplication(sys.argv)
        app.setStyle(QtWidgets.QStyleFactory.create("Fusion"))
        window = VTSWindow(client = client)
        window.show()
        sys.exit(app.exec_())

    

if __name__ == "__main__" :
    main()
