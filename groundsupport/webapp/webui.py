from flask_socketio import SocketIO, emit
from flask import Flask, render_template, url_for, copy_current_request_context
import time
import random
from threading import Thread, Event

__author__ = 'patriklaurell'

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
app.config['DEBUG'] = True

#turn the flask app into a socketio app
socketio = SocketIO(app)

thread = Thread()
thread_stop_event = Event()

class NewDataThread(Thread):
    def __init__(self):
        self.delay = 2
        super(NewDataThread, self).__init__()

    def randomNumberGenerator(self):
        """
        Generate a random number every 1 second and emit to a socketio instance (broadcast)
        Ideally to be run in a separate thread?
        """
        #infinite loop of magical random numbers
        while True:
            print("emitting newdata")
            socketio.emit('newdata', {'data': [random.randint(0,10) for i in range(5)]}, namespace='/test')
            time.sleep(self.delay)

    def run(self):
        self.randomNumberGenerator()


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
        thread = NewDataThread()
        thread.start()

if __name__ == "__main__":
    socketio.run(app)
