import socket
 
size = 8192
 
try:
  for i in range(51):
    msg = str(i).encode()
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(msg, ('localhost', 9876))
    print(bytes.decode(sock.recv(size)))
    sock.close()
 
except:
  print("cannot reach the server")