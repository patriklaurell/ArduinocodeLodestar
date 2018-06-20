import sys
import argparse
import socket
from flask_socketio import SocketIO, emit
from flask import Flask, render_template, url_for, copy_current_request_context
import time
import random
from threading import Thread, Event

__author__ = 'patriklaurell'

UDP_IP = "192.168.0.2"
UDP_PORT = 8888

DATA_FRAME_LEN = 88

def decode_packet(packet):
    time = int.from_bytes(packet[0:2], byteorder='big')
    frame_nr = int.from_bytes(packet[2:4], byteorder='big')
    rad = int.from_bytes(packet[4:6], byteorder='big')
    temp = int.from_bytes(packet[6:8], byteorder='big')

    measurements = [ [], [], [], [] ]
    current_index = 8
    for i in range(4):
        for j in range(10):
            x = packet[current_index:current_index+2]
            measurements[i].append(int.from_bytes(x, byteorder='big'))
            current_index += 2

    return {
        'v1':       measurements[0],
        'i1':       measurements[1],
        'v2':       measurements[2],
        'i2':       measurements[3],
        'temp':     temp,
        'time':     time,
        'frame':    frame_nr,
        'rad':      rad
    }


####### WEBAPP #######

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
app.config['DEBUG'] = True

#turn the flask app into a socketio app
socketio = SocketIO(app)

thread = Thread()
thread_stop_event = Event()

class RecieveDataThread(Thread):
    def __init__(self):
        super(RecieveDataThread, self).__init__()

    def udpDataListener(self):
        """
        Listen for data over UDP connection
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((UDP_IP, UDP_PORT))
        while True:
            data_packet, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
            print("New data recieved")
            data = decode_packet(data_packet)
            print("emitting newdata event")
            socketio.emit('newdata', {'data': data}, namespace='/test')

    def run(self):
        self.udpDataListener()

@app.route('/')
def index():
    #only by sending this page first will the client be connected to the socketio instance
    return render_template('index.html')

@socketio.on('connect')
def test_connect():
    # need visibility of the global thread object
    global thread
    print('Client connected')

    #Start the random number generator thread only if the thread has not been started before.
    if not thread.isAlive():
        print("Starting Thread")
        thread = RecieveDataThread()
        thread.start()




def main():
    parser = argparse.ArgumentParser(description="Lodestar BEXUS ground support program.")
    parser.add_argument('-f', '--file')
    parser.add_argument('--nogui')
    args = parser.parse_args()

    if args.file is not None:
        f = open(args.file, 'rb')
        try:
            packet = f.read(DATA_FRAME_LEN)
            while not all(x==0 for x in packet):
                decode_packet(packet)
                packet = f.read(88)
        finally:
            f.close()
    else:
        with open("data.lodestar", 'wb') as output:
            if args.nogui is None:
                print("Starting webapp...")
                socketio.run(app)

if __name__ == "__main__":
    main()
