#!/usr/bin/env python3
import sys, os
from peripherals import camera_comm
from PySide2 import QtCore

def main() :

    app = QtCore.QCoreApplication()
    camera_sim = camera_comm.CameraEmulator()
    camera_sim.start()
    return app.exec_()

if __name__ == "__main__" :
    main()
