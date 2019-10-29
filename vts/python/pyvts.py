#!/usr/bin/env python3
from __future__ import print_function

import time

#
# cli
#
import click

#
# vts
#
from vts_test import vts_loader

import argparse
import psutil

#
# Qt
#
from PySide2 import QtCore, QtGui, QtWidgets, QtNetwork
import json
import sys, os
import subprocess

import logging
logging.basicConfig(filename = "vts_run.log",
            filemode = "w",
            format = "%(asctime)s  %(levelname)s  %(message)s",
            datefmt = "%m/%d/%y %I:%M:%S %p",
            level = logging.DEBUG
)
logger = logging.getLogger("pyvts")
file_handler = logging.FileHandler("vts_run.log", mode = "w")
console_handler = logging.StreamHandler()
formatter = logging.Formatter("[%(asctime)s]  %(levelname)s  %(message)s",
                                datefmt = "%m/%d/%y %I:%M:%S")
file_handler.setFormatter(formatter)
console_handler.setFormatter(formatter)
logger.addHandler(file_handler)
logger.addHandler(console_handler)

#vts_loader.hello()

def check_running_vts() :

    procs = []
    pids = []
    for proc in psutil.process_iter() :
        try :
            pinfo = proc.as_dict(attrs = ['pid', 'name'])
            if 'vts' in pinfo['name'].strip().split('/')[-1] :
                print("FOUND PROC %s  PID %d" % (pinfo['name'], pinfo['pid']))
                procs.append(pinfo['name'])
                pids.append(pinfo['pid'])
        except :
            pass
    print("PROCS = %s" % procs)
    print("PIDS  = %s" % pids)
    return procs, pids

class VTSClient(QtCore.QObject) :
    def __init__(self, parent = None) :
        super(VTSClient, self).__init__(parent)
        self.tcpSocket = QtNetwork.QTcpSocket(self)

    def killServer(self) :
        logger.info("VTSClient Sending server KILL command")
        self.tcpSocket.abort()
        self.tcpSocket.connectToHost("127.0.0.1", 1234)
        #self.tcpSocket.connectToHost(self.hostLineEdit.text(),
        #        int(self.portLineEdit.text()))
        data_string="EXIT"
        byte_arr = bytearray(data_string, encoding = "utf-8")
        block = QtCore.QByteArray(byte_arr)
        self.tcpSocket.write(block)
        self.tcpSocket.waitForBytesWritten()

    def pingServer(self) :
        logger.info("VTSClient pinging server")
        self.tcpSocket.abort()
        self.tcpSocket.connectToHost("127.0.0.1", 1234)
        if not self.tcpSocket.waitForConnected(3000) :
            return False
        return True

    def startServer(self) :
        logger.info("VTSClient starting server")
        
        vts="/Users/dantrim/workarea/NSW/vmm_testing/vmm_testing_software/vts/build/vts"
        cmd = "%s" % vts
        server_pid = subprocess.Popen([vts, "&"])
        logger.info("server started in process %s" % server_pid.pid)
        return server_pid

class Client(QtWidgets.QDialog) :
    def __init__(self, parent = None) :
        super(Client, self).__init__(parent)

        self.blockSize = 0
        self.currentFortune = ''
        self.tcpSocket = QtNetwork.QTcpSocket(self)

        #hostLabel = QtWidgets.QLabel("&Server name:")
        #portLabel = QtWidgets.QLabel("S&erver port:")

        #self.hostLineEdit = QtWidgets.QLineEdit('127.0.0.1')
        #self.portLineEdit = QtWidgets.QLineEdit()
        #self.portLineEdit.setValidator(QtGui.QIntValidator(1, 65535, self))

        #hostLabel.setBuddy(self.hostLineEdit)
        #portLabel.setBuddy(self.portLineEdit)

        #self.statusLabel = QtWidgets.QLabel("This examples requires that you run "
        #        "the Fortune Server example as well.")

        #self.getFortuneButton = QtWidgets.QPushButton("Get Fortune")
        #self.getFortuneButton.setDefault(True)
        #self.getFortuneButton.setEnabled(False)

        #quitButton = QtWidgets.QPushButton("Quit")

        #startButton = QtWidgets.QPushButton("Start")

        #buttonBox = QtWidgets.QDialogButtonBox()
        #buttonBox.addButton(self.getFortuneButton,
        #        QtWidgets.QDialogButtonBox.ActionRole)
        #buttonBox.addButton(quitButton, QtWidgets.QDialogButtonBox.RejectRole)
        #buttonBox.addButton(startButton, QtWidgets.QDialogButtonBox.ActionRole)


        #self.hostLineEdit.textChanged.connect(self.enableGetFortuneButton)
        #self.portLineEdit.textChanged.connect(self.enableGetFortuneButton)
        #self.getFortuneButton.clicked.connect(self.requestNewFortune)
        #quitButton.clicked.connect(self.killServer)
        ##quitButton.clicked.connect(self.close)
        #startButton.clicked.connect(self.startServer)
        #self.tcpSocket.readyRead.connect(self.readFortune)
        #self.tcpSocket.error.connect(self.displayError)

        #mainLayout = QtWidgets.QGridLayout()
        #mainLayout.addWidget(hostLabel, 0, 0)
        #mainLayout.addWidget(self.hostLineEdit, 0, 1)
        #mainLayout.addWidget(portLabel, 1, 0)
        #mainLayout.addWidget(self.portLineEdit, 1, 1)
        #mainLayout.addWidget(self.statusLabel, 2, 0, 1, 2)
        #mainLayout.addWidget(buttonBox, 3, 0, 1, 2)
        #self.setLayout(mainLayout)

        #self.setWindowTitle("Fortune Client")
        #self.portLineEdit.setFocus()

    def killServer(self) :
        logger.info("Sending server KILL command")
        self.tcpSocket.abort()
        self.tcpSocket.connectToHost("127.0.0.1", 1234)
        #self.tcpSocket.connectToHost(self.hostLineEdit.text(),
        #        int(self.portLineEdit.text()))
        data_string="EXIT"
        byte_arr = bytearray(data_string, encoding = "utf-8")
        block = QtCore.QByteArray(byte_arr)
        self.tcpSocket.write(block)

    def pingServer(self) :
        logger.info("Pinging VTS server")
        self.tcpSocket.abort()
        self.tcpSocket.connectToHost("127.0.0.1", 1234)
        #self.tcpSocket.connectToHost(self.hostLineEdit.text(),
        #        int(self.portLineEdit.text()))
        if not self.tcpSocket.waitForConnected(1000) :
            return False
        return True

    def startServer(self) :
        logger.info("Sending server UP command")
        vts="/Users/dantrim/workarea/NSW/vmm_testing/vmm_testing_software/vts/build/vts"
        cmd = "%s &" % vts
        os.system(cmd)

    def requestNewFortune(self):
        #self.getFortuneButton.setEnabled(False)
        self.blockSize = 0
        self.tcpSocket.abort()
        self.tcpSocket.connectToHost(self.hostLineEdit.text(),
                int(self.portLineEdit.text()))
        block = QtCore.QByteArray()
        block.clear()
        out = QtCore.QDataStream(block, QtCore.QIODevice.WriteOnly)
        out.setVersion(QtCore.QDataStream.Qt_5_12)

        json_data = {
            "One": "Hello",
            "Two": "World",
            "sdp" : 1023,
            "gain" : 4.23,
            "peaking_time" : [43.2,2,3],
            "channels" : {
                "inner0" : 13,
                "inner1" : "lolz"
            }
        }
        data_string = json.dumps(json_data, ensure_ascii = True)
        byte_str = bytearray(data_string, encoding = "utf-8")
        block_arr = QtCore.QByteArray(byte_str)
        self.tcpSocket.write(block_arr)

    def readFortune(self):
        instr = QtCore.QDataStream(self.tcpSocket)
        instr.setVersion(QtCore.QDataStream.Qt_4_0)

        if self.blockSize == 0:
            if self.tcpSocket.bytesAvailable() < 2:
                return

            self.blockSize = instr.readUInt16()

        if self.tcpSocket.bytesAvailable() < self.blockSize:
            return

        nextFortune = instr.readString()

        try:
            # Python v3.
            nextFortune = str(nextFortune, encoding='ascii')
        except TypeError:
            # Python v2.
            pass

        if nextFortune == self.currentFortune:
            QtCore.QTimer.singleShot(0, self.requestNewFortune)
            return

        self.currentFortune = nextFortune
        self.statusLabel.setText(self.currentFortune)
        self.getFortuneButton.setEnabled(True)

    def displayError(self, socketError):
        if socketError == QtNetwork.QAbstractSocket.RemoteHostClosedError:
            pass
        elif socketError == QtNetwork.QAbstractSocket.HostNotFoundError:
            QtWidgets.QMessageBox.information(self, "Fortune Client",
                    "The host was not found. Please check the host name and "
                    "port settings.")
        elif socketError == QtNetwork.QAbstractSocket.ConnectionRefusedError:
            QtWidgets.QMessageBox.information(self, "Fortune Client",
                    "The connection was refused by the peer. Make sure the "
                    "fortune server is running, and check that the host name "
                    "and port settings are correct.")
        else:
            QtWidgets.QMessageBox.information(self, "Fortune Client",
                    "The following error occurred: %s." % self.tcpSocket.errorString())

        self.getFortuneButton.setEnabled(True)

    def enableGetFortuneButton(self):
        self.getFortuneButton.setEnabled(bool(self.hostLineEdit.text() and
                self.portLineEdit.text()))

@click.group()
@click.option("--config", default = "Hello", 
        help = "VTS configuration", show_default = True)
def vts(config) :
    print("VTS config: %s" % config)
    app  = QtWidgets.QApplication(sys.argv)
    client = Client()
    #client.show()
    sys.exit(client.exec_())
    pass

server_pid = None

@click.command()
@click.option("--start/--stop", help="Start/stop the server")
@click.option("--ping", help="Ping the server to see if it is running", is_flag=True)
def server(start, ping) :

    global server_pid

    if start :
        if server_pid is not None :
            logging.warning("SERVER ALREADY RUNNING")
        logger.info("Sending server UP command")
        vts="/Users/dantrim/workarea/NSW/vmm_testing/vmm_testing_software/vts/build/vts"
        cmd = "%s" % vts
        server_pid = subprocess.Popen([vts, "&"])
        logger.info("server started in process %s" % server_pid.pid)
        #os.system(cmd)

    if ping :
        logger.info("Pinging VTS server")
        ping_socket = QtNetwork.QTcpSocket()
        ip = QtNetwork.QHostAddress(QtNetwork.QHostAddress.LocalHost)
        ping_socket.connectToHost("127.0.0.1", 1234)
        connection_status = ping_socket.state()
        if ping_socket.waitForConnected(100) : #1000) :
            print("blah success")
            logging.info("PING SUCCESS -- CONNECTED")
        else :
            print("blah fail")
            logging.info("PING FAILED")
        
#        print("connection_status = %s" % connection_status)
#        if connection_status == QtNetwork.QAbstractSocket.SocketState.ConnectedState :
#            logger.info("PING OK")
#        else :
#            logger.info("PING FAILED")
        time.sleep(2)
        ping_socket.close()

    if not start and not ping :
        logger.info("Sending server KILL command")
        kill_socket = QtNetwork.QTcpSocket()
        kill_socket.abort() 
        #ip = QtNetwork.QHostAddress(QtNetwork.QHostAddress.LocalHost)
        ip = QtNetwork.QHostAddress(QtNetwork.QHostAddress.LocalHost)
        kill_socket.connectToHost("127.0.0.1", 1234)
        data_string = "EXIT"
        byte_arr = bytearray(data_string, encoding = "utf-8")
        block = QtCore.QByteArray(byte_arr)
        print("block = %s" % block)
        kill_socket.write(block)
        time.sleep(5)
        #kill_socket.close()

@click.command()
def gui() :
    print("gui")

vts.add_command(server)
vts.add_command(gui)

    

def main() :

    parser = argparse.ArgumentParser(description = "VTS")
    parser.add_argument("--gui", action = "store_true", default = False,
            help = "Open the GUI"
    )
    parser.add_argument("--comm", choices = ["START", "PING", "STOP", "CLEAN"], help = "Server commands")
#    parser.add_argument("--start", action = "store_true", default = False,
#            help = "Start the VTS server"
#    )
#    parser.add_argument("--stop", action = "store_true", default = False,
#            help = "Stop the VTS server"
#    )
#    parser.add_argument("--ping", action = "store_true", default = False,
#            help = "Ping the VTS server"
#    )

    args = parser.parse_args()

    app  = QtWidgets.QApplication(sys.argv)
    client = VTSClient()
    #client.show()

    server_pid = None

    procs, pids = check_running_vts()
    n_vts = len(procs)
    if n_vts > 0 :
        print("FOUND VTS")
        print(" --> %s" % (procs))
        print(" --> %s" % pids)

        if n_vts > 1 and args.comm != "CLEAN" :
            logging.critical("There appears to be more than one VTS server running!")
            sys.exit()
        if n_vts >= 1 and args.comm == "START" :
            print("There appears to already be a VTS server running, cannot start a new one!")
            sys.exit()

    if args.comm == "CLEAN" :
        for pid in pids :
            p = psutil.Process(pid)
            p.terminate()

    if args.comm == "PING" :
        ping_status = client.pingServer()
        if not ping_status :
            logger.critical("Unable to contact VTS server")
        else :
            logger.info("VTS server alive")

    if args.comm == "STOP" :
        client.killServer()

    if args.comm == "START" :
        client.startServer()


#    if args.gui :
#        client.show()
#    sys.exit(client.exec_())

    #vts()


if __name__ == "__main__" :
    main()
