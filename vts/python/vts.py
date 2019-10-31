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
        #print("{}".format(vts_client.ping_server()))
        vts_client.ping_server()
    elif args.server.lower() == "start" :
        vts_client.start_server()
    elif args.server.lower() == "stop" :
        vts_client.kill_server()
    elif args.server.lower() == "clean" :
        vts_client.clean()
    elif args.server.lower() == "dummy" :
        vts_client.dummy_send()

class VTSWindow(QtWidgets.QDialog) :
    def __init__(self, parent = None, client = None) :
        super(VTSWindow, self).__init__(parent)


        self.vts_client = client

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

        layout = QtWidgets.QGridLayout()
        layout.addWidget(button_box)
        self.setLayout(layout)

        self.setWindowTitle("VTS")
        self.start_button.setFocus()

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
        window = VTSWindow(client = client)
        window.show()
        sys.exit(window.exec_())
        
        

if __name__ == "__main__" :
    main()
