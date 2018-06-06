import sys
import argparse
import socket

UDP_IP = "192.168.0.2"
UDP_PORT = 8888

DATA_FRAME_LEN = 88

def handle_packet(packet):
  time = int.from_bytes(packet[0:2], byteorder='big')
  frame_nr = int.from_bytes(packet[2:4], byteorder='big')
  measurements = [ [], [], [], [] ]
  current_index = 4
  for i in range(4):
    for j in range(10):
      x = packet[current_index:current_index+2]
      measurements[i].append(int.from_bytes(x, byteorder='big'))
      current_index += 2
  rad = int.from_bytes(packet[current_index:current_index+2], byteorder='big')
  current_index +=2
  temp = int.from_bytes(packet[current_index:current_index+2], byteorder='big')

  print("time: {} \t frame: {}".format(time, frame_nr))
  print("radiation: {} \t temperature: {}".format(rad, temp))
  print("v1: {} \ni1: {} \nv2: {} \ni2: {} \n".format(
    measurements[0],
    measurements[1],
    measurements[2],
    measurements[3])) 


def main():
  parser = argparse.ArgumentParser(description="Lodestar BEXUS ground support program.")
  parser.add_argument('-f', '--file')
  args = parser.parse_args()

  if args.file is not None:
    f = open(args.file, 'rb')
    try:
      packet = f.read(DATA_FRAME_LEN)
      while not all(x==0 for x in packet):
        handle_packet(packet)
        packet = f.read(88)
    finally:
      f.close()
  else:
    with open("data.lodestar", 'wb') as output:
      sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
      sock.bind((UDP_IP, UDP_PORT))
      while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        handle_packet(data)
        output.write(data)

if __name__ == "__main__":
  main()
