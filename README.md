# Lodestar BEXUS

## Ground Station

The ground station control panel for Lodestar BEXUS consists of a Python backend and a HTML/JavaScript frontend. The back- and frontend communicate using SocketIO.

### Python backend
The backend has only been tested using Python 3.6. Using this version of Python is suggested. By default the backend listens for UDP packages on port 8888. To function with the Arduino code the ground station needs to be assigned th IP-adress 192.168.0.3.

```
$ python3 ground_station.py
```
