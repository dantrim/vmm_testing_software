#!/usr/bin/env python3

import sys, os
from pathlib import Path
import argparse
import json
import subprocess
import time


# VTS
from interface import vts_mainwindow
from vts_utils import vts_helpers, vts_client

# Qt
from PySide2 import QtCore, QtGui, QtWidgets, QtNetwork
from PySide2.QtCore import Slot, Signal

#ui->monitoring_enable_button->setStyleSheet("QPushButton {background-color: rgb(15, 147, 255);}");

VTS_GREEN = "(37, 229, 94)"
VTS_WHITE = "(255, 255, 255)"
VTS_RED = "(233, 40, 56)"
VTS_BLUE = "(84, 163, 252)"
VTS_GREY = "(189, 189, 189)"

class VTSWindow(QtWidgets.QMainWindow) :
    def __init__(self, client = None) :
        super(VTSWindow, self).__init__()
        self.vts = client
        self.ui = vts_mainwindow.Ui_MainWindow()
        self.ui.setupUi(self)
        self.setup_vts_connections(ui = self.ui)
        self.setup_defaults(ui = self.ui)


        self.signal_vts_config_updated.connect(self.signal_emitted)

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

    ##
    ## VTS SLOTS
    ##

    @Slot(str)
    def vmm_sn_updated(self, value) :
        bkg_color = "rgb{};".format(VTS_GREY)
        if value == "" :
            bkg_color = "rgb{};".format(VTS_RED)
        else :
            bkg_color = "rgb{};".format(VTS_GREEN)
        bkg_color = "QLabel {background-color: %s}" % bkg_color
        self.ui.label_vts_vmm_sn.setStyleSheet(bkg_color)
        self.ui.label_vts_vmm_sn.setText(str(value))

    @Slot(str)
    def server_status_updated(self, value) :
        if value not in ["Alive", "Dead"] :
            bkg_color = "rgb{};".format(VTS_GREY)
        else :
            bkg_color = { "Alive" : VTS_GREEN, "Dead" : VTS_RED }[value]
            bkg_color = "rgb{};".format(bkg_color)
        bkg_color = "QLabel {background-color: %s}" % bkg_color
        self.ui.label_vts_server_status.setStyleSheet(bkg_color)
        self.ui.label_vts_server_status.setText(str(value))

    @Slot(int)
    @Slot(str)
    def vts_print(self, value) :
        print("Sender: {}".format(self.sender().objectName()))
        self.setup_defaults(self.ui)
        self.signal_vts_config_updated.emit()
        
    @Slot()
    def signal_emitted(self) :
        print("-> Signal Emitted!")

    @Slot(str)
    def load_test_config_from_dir(self, value) :

        # don't let the user do this if the server isn't alive
        server_running = self.vts.ping_server(quiet = True)
        if not server_running :
            self.set_background(obj = self.ui.lineEdit_test_dir
                    ,obj_type_str = "QLineEdit"
                    ,reset = True
            )
            return

        test_config_dir = str(value)
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
        test_dict = vts_helpers.tests_from_test_dir(p_test_config_dir)

        if len(test_dict) >= 0 :
            reset = len(test_dict) == 0
            self.set_background(obj = self.ui.lineEdit_test_dir
                                ,obj_type_str = "QLineEdit"
                                ,color = VTS_GREEN
                                ,reset = reset
            )

        for test_name, test_config_file in test_dict.items() :
            self.ui.listWidget_loaded_tests.addItem(test_name)

        # assumes that VTS server is running
        if server_running :
            self.send_tests_to_vts()

    @Slot()
    def load_test_config(self) :

        self.clear_tests()
        test_config_dir = self.ui.lineEdit_test_dir.text()
        self.load_test_config_from_dir(str(test_config_dir))


    def get_loaded_tests(self) :

        test_names = []
        test_configs = []
        current_test_dir = self.ui.lineEdit_test_dir.text()


        n_items = self.ui.listWidget_loaded_tests.count()
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

    def send_tests_to_vts(self) :

        test_names, test_config_files = self.get_loaded_tests()
        self.vts.load_test(test_names = test_names, test_config_files = test_config_files)
        return

    ##
    ## VTS CONNECTIONS
    ##

    def setup_vts_connections(self, ui = None) :

        ##
        ## VTS CONTROL
        ##
        ui.button_start_vts_server.clicked.connect(self.vts.start_server)
        ui.button_shutdown_vts_server.clicked.connect(self.vts.kill_server)
        ui.button_ping_vts_server.clicked.connect(self.vts.ping_server)

        self.vts.signal_vmm_sn_updated.connect(self.vmm_sn_updated)
        self.vts.signal_server_status_updated.connect(self.server_status_updated)

        ##
        ## DEVICE CONTROL
        ##
        ui.button_acquire_vmm_serial.clicked.connect(self.vts.capture_vmm_serial)       
        ui.button_vts_config_check.clicked.connect(self.vts_print)

        ##
        ## TESTS
        ##
        ui.button_tests_load.clicked.connect(self.load_test_config) #vts.load_test)
        ui.button_tests_start.clicked.connect(self.vts.start_test)
        ui.button_tests_stop.clicked.connect(self.vts.stop_test)

        ##
        ## FPGA
        ##
        ui.button_fpga_configure.clicked.connect(self.vts.configure_fpga)
        ui.button_fpga_ping.clicked.connect(self.vts.ping_fpga)

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

    def setup_defaults(self, ui = None) :

        ##
        ## VTS CONTROL
        ##

        # default vts config
        default_vts_config = vts_helpers.default_vts_config()
        ui.lineEdit_vts_config_file.setText(str(default_vts_config))

        ##
        ## TESTING
        ##

        # tests
        ui.lineEdit_test_dir.clear()
        ui.listWidget_loaded_tests.clear()
        default_test_dir = vts_helpers.default_tests_dir()
        self.load_test_config_from_dir(str(default_test_dir))
#        test_dir, defined_tests = vts_helpers.get_defined_tests()
#        ui.lineEdit_test_dir.setText(str(test_dir))
#        for test_name, test_config_file in defined_tests.items() :
#            ui.listWidget_loaded_tests.addItem(test_name)

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
    elif cmd_time == "clean" :
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
