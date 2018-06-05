import sys

f = open(sys.argv[1], 'rb')

try:
  x = f.read(2)
  while x != b"":
    x = f.read(2)
    print(x)
    print(int.from_bytes(x, byteorder='little'))
    print() 

finally:
  f.close()
