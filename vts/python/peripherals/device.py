#!/usr/bin/env python3

import sys, os
import socket
import time

#CAM_IP = "127.0.0.1"
CAM_IP = "128.141.214.238"
CMD_PORT = 2005
SER_PORT = 2006
SERIAL_NUMBER = 51367

def close_socket(socket, how) :
    try :
        socket.shutdown(how)
        socket.close()
    except :
        return

def send(data = "", addr = ()) :

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
    sock.connect(addr)
    data = bytearray(data, encoding = "utf-8")
    sock.sendall(data)
    close_socket(sock, socket.SHUT_WR)

class Camera() :

    def __init__(self) :

        self.listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)

    def start(self) :


        # bind the sockets to their respective ports
        listen_address = (CAM_IP, CMD_PORT)
        #self.listen_socket.bind(listen_address)

        serial_address = (CAM_IP, SER_PORT)

        # start listening with the listen socket
        self.listen_socket.listen()
        print("Starting Camera Device: Listening on port {}".format(listen_address))
        try :
            while True :
                # wait for connection
                conn, addr = self.listen_socket.accept()
                with conn :
                    cmd_data = str(conn.recv(3), "utf-8")
                    if not cmd_data :
                        break
                    if cmd_data == "TRG" :
                        serial_num = "|-{}-|".format(SERIAL_NUMBER)
                        send(serial_num, serial_address)
                    elif "X" in cmd_data :
                        reply = "OK"
                        send(reply, serial_address)
                        break
            
            print("Shutting down camera")
            close_socket(self.listen_socket, socket.SHUT_RD)
            return
        except :
            print("Shutting down camera")
            close_socket(self.listen_socket, socket.SHUT_RD)

def main() :

    if sys.argv[1].lower() == "cam" :
        camera = Camera()
        camera.start()

if __name__ == "__main__" :
    main()
