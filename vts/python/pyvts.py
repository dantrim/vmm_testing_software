#!/bin/env python
from __future__ import print_function

#
# Qt
#
from PySide2 import QtCore, QtGui, QtWidgets, QtNetwork
import json

class Client(QtWidgets.QDialog) :
    def __init__(self, parent = None) :
        super(Client, self).__init__(parent)

        self.blockSize = 0
        self.currentFortune = ''

        hostLabel = QtWidgets.QLabel("&Server name:")
        portLabel = QtWidgets.QLabel("S&erver port:")

        self.hostLineEdit = QtWidgets.QLineEdit('127.0.0.1')
        self.portLineEdit = QtWidgets.QLineEdit()
        self.portLineEdit.setValidator(QtGui.QIntValidator(1, 65535, self))

        hostLabel.setBuddy(self.hostLineEdit)
        portLabel.setBuddy(self.portLineEdit)

        self.statusLabel = QtWidgets.QLabel("This examples requires that you run "
                "the Fortune Server example as well.")

        self.getFortuneButton = QtWidgets.QPushButton("Get Fortune")
        self.getFortuneButton.setDefault(True)
        self.getFortuneButton.setEnabled(False)

        quitButton = QtWidgets.QPushButton("Quit")

        buttonBox = QtWidgets.QDialogButtonBox()
        buttonBox.addButton(self.getFortuneButton,
                QtWidgets.QDialogButtonBox.ActionRole)
        buttonBox.addButton(quitButton, QtWidgets.QDialogButtonBox.RejectRole)

        self.tcpSocket = QtNetwork.QTcpSocket(self)

        self.hostLineEdit.textChanged.connect(self.enableGetFortuneButton)
        self.portLineEdit.textChanged.connect(self.enableGetFortuneButton)
        self.getFortuneButton.clicked.connect(self.requestNewFortune)
        quitButton.clicked.connect(self.close)
        self.tcpSocket.readyRead.connect(self.readFortune)
        self.tcpSocket.error.connect(self.displayError)

        mainLayout = QtWidgets.QGridLayout()
        mainLayout.addWidget(hostLabel, 0, 0)
        mainLayout.addWidget(self.hostLineEdit, 0, 1)
        mainLayout.addWidget(portLabel, 1, 0)
        mainLayout.addWidget(self.portLineEdit, 1, 1)
        mainLayout.addWidget(self.statusLabel, 2, 0, 1, 2)
        mainLayout.addWidget(buttonBox, 3, 0, 1, 2)
        self.setLayout(mainLayout)

        self.setWindowTitle("Fortune Client")
        self.portLineEdit.setFocus()

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
        data_string = json.dumps(json_data, ensure_ascii = True) #encoding="utf-8")
        out.writeString(data_string)
        idx_first = list(block).index(b"{")
        idx_last = len(list(block)) - 1 - list(block)[::-1].index(b"}")
        block = block[idx_first:idx_last+1]
        self.tcpSocket.write(block)



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


def main() :

    import sys
    app  = QtWidgets.QApplication(sys.argv)
    client = Client()
    client.show()
    sys.exit(client.exec_())
if __name__ == "__main__" :
    main()
