#!/usr/bin/env python3

# A simple program used to reverse the obfuscation
# of deadmeat.swf (https://www.deadmaze.com/alpha/deadmeat.swf)

import os
import stat
import zlib
from sys import argv
from urllib.request import urlopen
from tempfile import TemporaryDirectory

from typing import List

FFDEC_EXE = os.getenv('FFDEC_EXE', 'ffdec')
OUTPUT_FILE = argv[1] if len(argv) >= 2 else 'reversed.swf'
loader_url = 'https://www.deadmaze.com/alpha/deadmeat.swf'


def exportBinaryData(path: str) -> List[str]:
    deadmeat = f"{path}/deadmeat.swf"

    with urlopen(loader_url) as url:
        if url.getcode() != 200:
            raise Exception("Can't DL deadmeat.swf")

        with open(deadmeat, 'wb') as f:
            f.write(url.read())

    os.system(f"{FFDEC_EXE} -export binarydata '{path}/data' '{deadmeat}'")

    return [os.path.join(path, 'data', i) for i in os.listdir(f"{path}/data")]


def loadFilesInMemory() -> List[bytes]:
    global verbose
    result = []

    with TemporaryDirectory(prefix='bloader_') as tmpdir:
        for pathname in exportBinaryData(tmpdir):
            mode = os.stat(pathname).st_mode
            if stat.S_ISREG(mode):
                if verbose:
                    print(f"[*] Loading in memory: {pathname}")
            with open(pathname, 'rb') as f:
                result.append(f.read())

    return result


def findHead() -> bool:
    global head

    # Find the Head
    for i in range(0, len(binaryData)):
        if binaryData[i][0:3] == b'CWS':
            head = binaryData[i]
            binaryData.pop(i)
            break
    if head == b'':
        print("[-] Couldn't find the CWS header in all bin Data...")
        return False

    return True


def worker() -> None:
    global head, cwsheader
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

            if not exception or "Error -5" in exception.args[0]:
                prev_content = content
                binaryData.pop(i)
                break

    print(f"[+] Writing {OUTPUT_FILE}...")
    with open(OUTPUT_FILE, 'wb') as f:
        f.write(cwsheader)
        f.write(prev_content)


if __name__ == "__main__":
    verbose = True
    head = b''
    cwsheader = b''

    binaryData = loadFilesInMemory()

    if not findHead():
        exit("[-] Something fucked up, exiting")

    print("[*] Trying to retrieve stream order...")
    cwsheader = head[:8]
    head = head[8:]  # removing header, keeping ZLIB info only

    worker()
    print("[+] All done, have a nice day o/")
