import socket

size = 8192

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

seq = 1

try:
  while True:
    data, address = sock.recvfrom(size)
    msg = data.decode()
    msg = (str(seq) + ' ' + msg).encode()
    sock.sendto(msg, address)
    seq += 1
finally:
  sock.close()