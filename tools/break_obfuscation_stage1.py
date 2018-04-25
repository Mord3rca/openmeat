#!/usr/bin/env python3

import os, stat, sys
import zlib

verbose = True
head, tail = [b'', b'']
binaryData = []
cwsheader = b''

def loadFilesInMemory(path):
  global binaryData, verbose

  for f in os.listdir(path):
    pathname = os.path.join(path, f)
    mode = os.stat(pathname).st_mode
    if stat.S_ISREG(mode):
      if verbose:
        print("[*] Loaded in memory: " + pathname)
      with open(pathname, 'rb') as f:
        binaryData.append( f.read() )

def findHeadAndTail():
  global head, tail

  #Find the Head
  for i in range(0, len(binaryData)):
    if( binaryData[i][0:3] == b'CWS' ):
      head = binaryData[i]
      binaryData.pop(i)
      break
  if( head == b'' ):
    print( "[-] Couldn't find the CWS header in all bin Data..." )
    return False

  #Find the Tail
  binLen, pos = [0,0]
  for i in range(0, len(binaryData)):
    if( len(binaryData[i]) > binLen ):
      binLen = len(binaryData[i])
      tail = binaryData[i]
      pos = i
  binaryData.pop(pos)
  return True

def worker():
  global head, cwsheader, tail
  prev_content = head

  while len(binaryData) > 0:
    for i in range(0, len(binaryData)):
      exception = ''
      content = prev_content + binaryData[i]
      try:
        zlib.decompress(content)
      except zlib.error as err:
        exception = err
        pass
      
      if "Error -5" in exception.args[0]:
        prev_content = content
        binaryData.pop(i)
        break
     
  with open("/tmp/reversed.swf", 'wb') as f:
    f.write(cwsheader)
    f.write(prev_content)
    f.write(tail)

if __name__=="__main__":
  loadFilesInMemory(sys.argv[1])
  
  if not findHeadAndTail():
    print("[-] Something fucked up, exiting")
    exit()

  print("[*] Trying to retrieve stream order...")
  cwsheader = head[:8]
  head = head[8:] #removing header, keeping ZLIB info only

  worker()
  print("[+] All done, have a nice day o/")
