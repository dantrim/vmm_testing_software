#!/usr/bin/env python3

import sys
from pathlib import Path

# VTS
from interface import vts_mainwindow
from vts_utils import vts_helpers

# Qt
from PySide2 import QtCore, QtGui, QtWidgets, QtNetwork
from PySide2.QtCore import Slot, Signal

class VTSWindow(QtWidgets.QMainWindow) :
    def __init__(self) :
        super(VTSWindow, self).__init__()
        self.ui = vts_mainwindow.Ui_MainWindow()
        self.ui.setupUi(self)
        self.setup_vts_connections(ui = self.ui)
        self.setup_defaults(ui = self.ui)


        self.signal_vts_config_updated.connect(self.signal_emitted)
    ##
    ## VTS SIGNALS
    ##
    signal_vts_config_updated = Signal()

    ##
    ## VTS SLOTS
    ##

    @Slot(int)
    @Slot(str)
    def vts_print(self, value) :
        print("Sender: {}".format(self.sender().objectName()))
        self.setup_defaults(self.ui)
        self.signal_vts_config_updated.emit()
        
    @Slot()
    def signal_emitted(self) :
        print("-> Signal Emitted!")

    ##
    ## VTS CONNECTIONS
    ##

    def setup_vts_connections(self, ui = None) :

        ui.button_vts_config_check.clicked.connect(self.vts_print)

    ##
    ## VTS CONNECTIONS
    ##

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
        test_dir, defined_tests = vts_helpers.get_defined_tests()
        ui.lineEdit_test_dir.setText(str(test_dir))
        for test_name, test_config_file in defined_tests.items() :
            ui.listWidget_loaded_tests.addItem(test_name)

def main() :

    app = QtWidgets.QApplication(sys.argv)
    app.setStyle(QtWidgets.QStyleFactory.create("Fusion"))
    window = VTSWindow()
    window.show()
    sys.exit(app.exec_())

    

if __name__ == "__main__" :
    main()
