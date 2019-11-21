#!/usr/bin/env python3
from __future__ import print_function

import argparse
import psutil
import json
import sys, os
import subprocess
import logging
import time

# Qt
from PySide2 import QtCore, QtGui, QtWidgets, QtNetwork

# vts
from vts_utils import vts_client
from peripherals import camera_comm

def args_ok(args) :

    """
    Check that the user command line arguments are OK
    (e.g. that the configuration file can be found, etc...)
    """

    ok = True
    if not os.path.isfile(args.config) :
        print("ERROR Cannot find specified VTS configuration file: {}".format(args.config))
        ok = False
    return ok

def server_command_and_exit(args, vts_client) :

    if args.server.lower() == "ping" :
        vts_client.ping_server(quiet = False)
    elif args.server.lower() == "start" :
        vts_client.start_server()
    elif args.server.lower() == "stop" :
        vts_client.kill_server()
    elif args.server.lower() == "clean" :
        vts_client.clean()
    elif args.server.lower() == "dummy" :
        vts_client.dummy_send()

class VTSWindow(QtWidgets.QDialog) :
    def __init__(self, parent = None, client = None, camera = None) :
        super(VTSWindow, self).__init__(parent)


        self.vts_client = client
        self.comm = camera

        ##
        ## server buttons
        ##

        self.start_button = QtWidgets.QPushButton("Start")
        self.stop_button = QtWidgets.QPushButton("Stop")
        self.ping_button = QtWidgets.QPushButton("Ping")
        self.dummy_button = QtWidgets.QPushButton("Dummy")

        button_box = QtWidgets.QDialogButtonBox()
        button_box.addButton(self.start_button, QtWidgets.QDialogButtonBox.ActionRole)
        button_box.addButton(self.stop_button, QtWidgets.QDialogButtonBox.ActionRole)
        button_box.addButton(self.ping_button, QtWidgets.QDialogButtonBox.ActionRole)
        button_box.addButton(self.dummy_button, QtWidgets.QDialogButtonBox.ActionRole)

        self.start_button.clicked.connect(self.vts_client.start_server)
        self.stop_button.clicked.connect(self.vts_client.kill_server)
        self.ping_button.clicked.connect(self.vts_client.ping_server)
        self.dummy_button.clicked.connect(self.vts_client.dummy_send)

        ##
        ## frontend buttons
        ##

        self.board_power_on_button = QtWidgets.QPushButton("Power On")
        self.board_power_off_button = QtWidgets.QPushButton("Power Off")
        self.ping_fpga_button = QtWidgets.QPushButton("Ping FPGA")
        self.configure_fpga_button = QtWidgets.QPushButton("Configure FPGA")
        self.acq_on_button = QtWidgets.QPushButton("ACQ On")
        self.acq_off_button = QtWidgets.QPushButton("ACQ Off")

        self.send_spi_button = QtWidgets.QPushButton("Send SPI")
        self.vmm_reset_button = QtWidgets.QPushButton("Reset VMM")

        buttons_frontend = QtWidgets.QDialogButtonBox()
        buttons_frontend.addButton(self.board_power_on_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_frontend.addButton(self.board_power_off_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_frontend.addButton(self.ping_fpga_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_frontend.addButton(self.configure_fpga_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_frontend.addButton(self.acq_on_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_frontend.addButton(self.acq_off_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_frontend.addButton(self.send_spi_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_frontend.addButton(self.vmm_reset_button, QtWidgets.QDialogButtonBox.ActionRole)

        self.board_power_on_button.clicked.connect(self.vts_client.board_on)
        self.board_power_off_button.clicked.connect(self.vts_client.board_off)
        self.ping_fpga_button.clicked.connect(self.vts_client.ping_fpga)
        self.configure_fpga_button.clicked.connect(self.vts_client.configure_fpga)
        self.acq_on_button.clicked.connect(self.vts_client.acq_on)
        self.acq_off_button.clicked.connect(self.vts_client.acq_off)
        self.send_spi_button.clicked.connect(self.vts_client.configure_vmm)
        self.vmm_reset_button.clicked.connect(self.vts_client.reset_vmm)

        ##
        ## test buttons
        ##
        self.load_test_button = QtWidgets.QPushButton("Load Test")
        self.start_test_button = QtWidgets.QPushButton("Start Test")
        self.stop_test_button = QtWidgets.QPushButton("Stop Test")
        buttons_test = QtWidgets.QDialogButtonBox()
        buttons_test.addButton(self.load_test_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_test.addButton(self.start_test_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_test.addButton(self.stop_test_button, QtWidgets.QDialogButtonBox.ActionRole)

        self.load_test_button.clicked.connect(self.vts_client.load_test)
        self.start_test_button.clicked.connect(self.vts_client.start_test)
        self.stop_test_button.clicked.connect(self.vts_client.stop_test)

        ##
        ## serial
        ##
        self.take_picture_button = QtWidgets.QPushButton("Take Picture")
        self.load_vmm_serial_button = QtWidgets.QPushButton("VMM SERIAL")
        buttons_serial = QtWidgets.QDialogButtonBox()
        buttons_serial.addButton(self.take_picture_button, QtWidgets.QDialogButtonBox.ActionRole)
        buttons_serial.addButton(self.load_vmm_serial_button, QtWidgets.QDialogButtonBox.ActionRole)
        self.take_picture_button.clicked.connect(self.take_vmm_picture)
        self.load_vmm_serial_button.clicked.connect(self.get_serial) #comm.request_serial) #vts_client.get_vmm_serial)


        ##
        ## layout
        ##

        layout = QtWidgets.QGridLayout()
        layout.addWidget(button_box)
        layout.addWidget(buttons_frontend)
        layout.addWidget(buttons_test)
        layout.addWidget(buttons_serial)
        self.setLayout(layout)

        self.setWindowTitle("VTS")
        self.start_button.setFocus()

    def take_vmm_picture(self) :
        self.comm.request_serial()
        return

    def get_serial_(self) :
        return self.comm.current_serial()

    def get_serial(self) :

        serial = self.comm.current_serial()
        if serial == "NULL" :
            print("Failed to obtain VMM serial number!")
            return
        print("Received VMM serial number: {}".format(serial))

def main() :

    ##
    ## load in cli
    ##
    parser = argparse.ArgumentParser(description = "VTS: VMM Testing Software")
    parser.add_argument("-c", "--config", default = "../config/vts_default.json",
            help = "VTS configuration"
    )
    parser.add_argument("--gui", action = "store_true", default = False,
            help = "Launch the graphical user interface"
    )
    parser.add_argument("-s", "--server",
            choices = ["START", "STOP", "PING", "CLEAN", "DUMMY"],
            help = "VTS Server Commands"
    )

    ##
    ## parse and check the args
    ##
    args = parser.parse_args()
    if not args_ok(args) :
        print("Unable to begin")
        sys.exit(1)

    ##
    ## load the VTS configuration
    ##
    with open(args.config) as config_file :
        config_data = json.load(config_file)
        if "vts_config" not in config_data :
            print("ERROR \"vts_config\" top-level is not found in provided config file: {}".format(args.config))
            sys.exit()
        config_data = config_data["vts_config"]
    vts_server_config, logging_config = config_data["vts_server"], config_data["logging"]

    ##
    ## instantiate the VTS client
    ##
    client = vts_client.VTSClient(config = vts_server_config, config_file = args.config)

    ##
    ## process commands
    ##
    if args.server :
        server_command_and_exit(args, client)
        sys.exit(0)
    if args.gui :
        app = QtWidgets.QApplication(sys.argv) 
        comm = camera_comm.CameraComm()
        comm.start()
        comm.connect_to_camera()
        window = VTSWindow(client = client, camera = comm)
        window.show()
        sys.exit(window.exec_())

if __name__ == "__main__" :
    main()
