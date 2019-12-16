#!/usr/bin/env python3

import sys, os
import socket
import re
from collections import defaultdict
from .device import CAM_IP, CMD_PORT, SER_PORT, close_socket, send

class PictureTaker() :
    def __init__(self) :

        self.listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
        self.serial_number = ""

    def shutoff_camera(self) :
        self.request(cmd = "X")

    def get_serial_number(self) :

        try :
            self.request(cmd = "TRG")
            return self.serial_number
        except :
            return ""

    def request(self, cmd = "TRG", attempts = 5) :

        cmd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        cmd.settimeout(2)
        cmd.connect(("128.141.214.238", 2006))

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)
        sock.connect(("128.141.214.238", 2005))

        results = []
        for _ in range(attempts):
            n = cmd.send(b"TRG")
            data = ""
            while True:
                chunk = sock.recv(1)
                if len(chunk) == 0:
                    print("Remote host has closed the connection.")
                    raise Exception("Connection is lost")

                data += chunk.decode()
                m = re.match(r"^\|-(\d+)-\|", data)
                if m is not None:
                    break
            #print(data)
            #print(m.group(1))

            results.append(m.group(1))

        cmd.close()
        sock.close()
        print("VISOR returns: {}".format(results))
        self.serial_number = results[0]
######

#        # make connection
#        listen_address = (CAM_IP, SER_PORT)
#        self.listen_socket.bind(listen_address)
#
#        cmd_address = (CAM_IP, CMD_PORT)
#
#        # start listening with the listen socket
#        self.listen_socket.listen()
#
#        # send the command
#        serial_number = ""
#        attempts = 0
#        try :
#            while True :
#                send(cmd, cmd_address)
#                conn, addr = self.listen_socket.accept()
#                with conn :
#                    ser_data = str(conn.recv(32), "utf-8")
#                    print("RECEIVED SER_DATA: {}".format(ser_data))
#                    if not ser_data :
#                        break
#                    if "X" in cmd and ser_data == "OK" :
#                        break
#                    elif cmd == "TRG" and ser_data != "" :
#                        if "|-" in ser_data and "-|" in ser_data :
#                            serial_number = ser_data.strip().replace("|-","").replace("-|","").strip()
#                        else :
#                            print("Unknown formatting for received serial number: {}".format(ser_data))
#                            serial_number = ""
#                        break
#                    else :
#                        serial_number = ""
#                        break
#            if serial_number == "" and "X" not in cmd :
#                print("Failed to receive VMM serial number")
#                serial_number = ""
#        except :
#            self.close()
#            return
#        self.close()
#
#        self.serial_number = serial_number

    def close(self) :
        close_socket(self.listen_socket, socket.SHUT_RD)

if __name__ == "__main__" :
    main()
