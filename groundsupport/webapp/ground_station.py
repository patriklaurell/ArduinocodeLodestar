import sys
import scipy.io as sio
import numpy as np
import argparse
import socket
from flask_socketio import SocketIO, emit
from flask import Flask, render_template, url_for, copy_current_request_context
import time
import random
from threading import Thread, Event

__author__ = 'patriklaurell'

UDP_IP = "172.16.18.130"
UDP_PORT = 8888

DATA_FRAME_LEN = 2058

frames = []
LIVE_MODE = True
FILE_NAME = ""
FRAME_NR = 0

def decode_packet(packet):
    if len(packet) == 8:
        time = int.from_bytes(packet[0:2], byteorder='big', signed=False)
        frame_nr = int.from_bytes(packet[2:4], byteorder='big', signed=False)
        rad = int.from_bytes(packet[4:6], byteorder='big', signed=False)
        temp = round(int.from_bytes(packet[6:8], byteorder='big', signed=False) * 125/(2**16) - 40, 1)
        return {
            'temp':     temp,
            'time':     time,
            'frame':    frame_nr,
            'rad':      rad
        }
    else:
        v = []
        i = []
        current_index = 0
        for j in range(256):
            v.append(int.from_bytes(packet[current_index:current_index+2], byteorder='big'))
            current_index += 2
            i.append(int.from_bytes(packet[current_index:current_index+2], byteorder='big'))
            current_index += 2
        cell = int.from_bytes(packet[current_index:current_index+2], byteorder='big')
        return {
            'v':       v,
            'i':       i,
            'cell_nr':  cell
        }

def savePacketToFile(packet):
    print("Writing packet to file...")
    with open('output.lodestar', 'ab+') as f:
        f.write(packet)
    print("...done!")

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
        global frames
        global FRAME_NR

        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((UDP_IP, UDP_PORT))
        while True:
            data_packet, addr = sock.recvfrom(1025) # buffer size is 1024 bytes
            print("New packet recieved")

            savePacketToFile(data_packet)
            
            if len(data_packet) == 8:
                data = decode_packet(data_packet)
                socketio.emit('new_metadata', {'data': data}, namespace='/test')
                frames.append({'metadata': data, 'data': []})
                FRAME_NR = len(frames) - 1
                print("Current frame number: {}".format(FRAME_NR))
            else:
                data = decode_packet(data_packet)
                print("Adding data to last frame.")
                frames[-1]['data'].append(data)
                print("emitting new_data event")
                socketio.emit('new_data', {'data': data}, namespace='/test')

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

    #Start the thread to recieve UDP packages.
    if not thread.isAlive() and LIVE_MODE:
        print("Running in live monitor mode.")
        print("Starting thread to recieve UDP packages... ", end='')
        thread = RecieveDataThread()
        thread.start()
        print("done!")
        print("Listening for UDP packages on port {}".format(UDP_PORT))

    elif not LIVE_MODE:
        print("Running in file inspect mode. Reading from file {}".format(FILE_NAME))

@socketio.on('next_frame', namespace='/test')
def next_frame():
    print("Recieved next frame event")
    global FRAME_NR
    global frames
    if FRAME_NR + 1 < len(frames):
        FRAME_NR += 1
        print("Current frame_nr = {}".format(FRAME_NR))
        print("frames[FRAME_NR] = ")
        print(frames[FRAME_NR])
        socketio.emit('new_metadata', {'data': frames[FRAME_NR]['metadata']}, namespace='/test')
        socketio.emit('new_data', {'data': frames[FRAME_NR]['data'][0]}, namespace='/test')
        socketio.emit('new_data', {'data': frames[FRAME_NR]['data'][1]}, namespace='/test')
    else:
        print("Current frame_nr = {}".format(FRAME_NR))
        print("Already at last frame.")

@socketio.on('prev_frame', namespace='/test')
def next_frame():
    print("Recieved prev frame event")
    global FRAME_NR
    global frames
    if FRAME_NR > 0:
        FRAME_NR -= 1
        print("Current frame_nr = {}".format(FRAME_NR))
        print("frames[FRAME_NR] = ")
        socketio.emit('new_metadata', {'data': frames[FRAME_NR]['metadata']}, namespace='/test')
        socketio.emit('new_data', {'data': frames[FRAME_NR]['data'][0]}, namespace='/test')
        socketio.emit('new_data', {'data': frames[FRAME_NR]['data'][1]}, namespace='/test')
    else:
        print("Current frame_nr = {}".format(FRAME_NR))
        print("Already at first frame.")

@socketio.on('goto_frame', namespace='/test')
def goto_frame(frame):
    global FRAME_NR
    print("Recieved goto frame: {}".format(frame))
    FRAME_NR = frame
    print("Current frame_nr = {}".format(FRAME_NR))
    print("Emitting new data events to client...", end='')
    socketio.emit('new_metadata', {'data': frames[FRAME_NR]['metadata']}, namespace='/test')
    socketio.emit('new_data', {'data': frames[FRAME_NR]['data'][0]}, namespace='/test')
    socketio.emit('new_data', {'data': frames[FRAME_NR]['data'][1]}, namespace='/test')
    print("done!")






def main():
    parser = argparse.ArgumentParser(description="Lodestar BEXUS ground support program.")
    parser.add_argument('-f', '--file')
    parser.add_argument('--nogui')
    parser.add_argument('-e', '--export')
    args = parser.parse_args()

    global FILE_NAME
    global LIVE_MODE

    # Read file mode
    if args.file is not None:
        FILE_NAME = args.file
        LIVE_MODE = False
        f = open(args.file, 'rb')
        try:
            print("Processing frames in file...")
            packet = f.read(DATA_FRAME_LEN)
            while not all(x==0 for x in packet):
                if len(frames) % 100 == 0:
                    print("Completed {} frames".format(len(frames)))
                metadata = decode_packet(packet[0:8])
                data1 = decode_packet(packet[8:1025+8])
                data2 = decode_packet(packet[1025+8:])
                frames.append({
                    'metadata': metadata,
                    'data': [data1, data2]
                })
                packet = f.read(DATA_FRAME_LEN)
            print("...done!")

        finally:
            f.close()

        socketio.run(app)

    elif args.export is not None:
        FILE_NAME = args.export
        LIVE_MODE = False
        print("Exporting data...")
        f = open(FILE_NAME, 'rb')
        try:
            print("Processing frames in file...")
            cells = [[],[],[],[],[],[]]
            temp = []
            time = []
            frame = []
            rad = []
            packet = f.read(DATA_FRAME_LEN)
            length = 0
            while not all(x==0 for x in packet):
                metadata = decode_packet(packet[0:8])
                data1 = decode_packet(packet[8:1025+8])
                data2 = decode_packet(packet[1025+8:])
                temp.append(metadata['temp'])
                frame.append(metadata['frame'])
                rad.append(metadata['rad'])
                time.append(metadata['time'])
                length += 1
                
                cell1 = data1['cell_nr']
                cell2 = data2['cell_nr']
                if cell1 >= 1 and cell1 <= 6:
                    cells[cell1-1].append([data1['v'], data1['i']])
                if cell2 >= 1 and cell2 <= 6:
                    cells[cell2-1].append([data2['v'], data2['i']])
                packet = f.read(DATA_FRAME_LEN)
            print("...done!")
        finally:
            print("Writing {} frames to iv_curves.mat...".format(length))
            sio.savemat('iv_curves.mat', {'cell'+str(i+1): np.array(cells[i], dtype='Float16') for i in range(6)})
            sio.savemat('metadata.mat', {'time': time, 'frame': frame, 'temp': temp, 'rad':rad})
            print("...done!")
            f.close()

    # Live monitor mode
    else:
        if args.nogui is None:
            print("Starting webapp...")
            socketio.run(app)

if __name__ == "__main__":
    main()
