#!/usr/bin/env python3

import sys, os
from peripherals import device_capture

if __name__ == "__main__" :

    camera = device_capture.PictureTaker()
    if "x" in sys.argv[1].lower() :
        camera.shutoff_camera()
    elif sys.argv[1].lower() == "s" :
        vmm_sn = camera.get_serial_number()
        print("Retrieved VMM serial number: {}".format(vmm_sn))
