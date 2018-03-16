#!/usr/bin/env python3

import _thread
import socket

def WorkerThread(msg, sockI, sockO):
  while 1:
    data = sockI.recv(2048)
    if not data:
      break;
    print("[" + msg + "] - " + " ".join("{:02x}".format(c) for c in data))
    sockO.send(data)
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
#Connection OK
#conn1.send(b'\x00\x5A\x11\x22\x33\x44')

_thread.start_new_thread(WorkerThread, ("OUT", conn1, s2, ))
_thread.start_new_thread(WorkerThread, ("IN" , s2, conn1, ))

while 1:
  pass

print("END")
conn1.close()
s2.close()
