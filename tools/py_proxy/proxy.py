#!/usr/bin/env python3

import _thread
import socket

def InputThread(sockI, sockO):
  while 1:
    data = sockI.recv(2048)
    if not data:
      break;
    #print("[IN] - " + " ".join("{:02x}".format(c) for c in data))
    sockO.send(data)
  return

def OutgoingThread(sockI, sockO):
  while 1:
    data = sockO.recv(2048)
    if not data:
      break
    print("[OUT] - " + " ".join("{:02x}".format(c) for c in data))
    sockI.send(data)
  return

#Server
s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s1.bind(('', 4444))
s1.listen(1)
conn1, addr1 = s1.accept()
print("Target hooked, connecting to DM server")

#Client
s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


print("Connected by " + str(addr1))
connectinfo = conn1.recv(8)
socks4 = ( str(connectinfo[4]) + '.' + str(connectinfo[5]) + '.' + str(connectinfo[6]) + '.' + str(connectinfo[7])
          , connectinfo[3]*0x100 + connectinfo[2])
s2.connect(socks4)

th1 = _thread.start_new_thread(InputThread, (conn1, s2, ))
th2 = _thread.start_new_thread(OutgoingThread, (conn1, s2, ))

#while 1:
#  datato = conn1.recv(4096)
#  if not datato:
#    break
#  s2.send(datato)
#  print("Data sent: " + str(datato))
#
#  try:
#    datafrom = s2.recv(4096)
#    if not datafrom:
#      break
#    print("DATA received: " + str(datafrom))
#  except socket.error as msg:
#    print("SOCK error: " + str(msg))
#    continue
#
#  conn1.send(datafrom)

while 1:
  pass

print("END")
conn1.close()
s2.close()
