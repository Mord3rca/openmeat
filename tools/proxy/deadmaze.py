import socket
import parser
import importlib

from itertools import cycle
from proxy import ProxyClient
from struct import pack, unpack
from typing import Tuple

community_key = bytes.fromhex('8a0142e1bdc5187f4017010ac1d50fd11c3ca5b1')


def community_encode(data: bytes, pos: int) -> bytes:
    k = community_key[pos:] + community_key[:pos]
    return bytes(
        [i ^ j for i, j in zip(data, cycle(k))]
    )


def community_decode(data: bytes, pos: int = -1) -> bytes:
    if pos == -1:
        pos = community_key.find(data[0])
    k = community_key[pos:] + community_key[:pos]
    return bytes(
        [i ^ j for i, j in zip(data, cycle(k))]
    )


class DeadMazePacket:

    def __init__(self, client=False) -> None:
        self.__raw = b''
        self.__opcode = 0
        self.__data = b''
        self.__from_client = client

        self._size = 0
        self._seek = 0

        self._complete = False

    def __str__(self):
        return f"""\
DeadMaze {'Client' if self.__from_client else 'Server'} Packet
OpCode: {hex(self.opcode)}
Data:   {self.__data}"""

    @staticmethod
    def read_header_size(b: bytes, client: bool) -> Tuple[int, bytes]:
        seq = 0
        i = 0
        size = 0

        while i < 4:
            size += (b[i] & 0x7F) << (0 if i == 0 else i*7)

            if not b[i] & 0x80:
                break

            i += 1

        i += 1

        if client:
            seq = b[i]
            i += 1

        return size, seq, b[i:]

    @staticmethod
    def encode_header_size(size: int) -> bytes:
        d = bytearray()
        for i in range(4):
            d += int.to_bytes(size & 0x7f)
            size >>= 7
            if size != 0:
                d[i] += 0x80
            else:
                break

        if size != 0:
            raise Exception("Nope !")

        return bytes(d)

    def write(self, data: bytes) -> bytes:
        if self._size == 0:
            self._size, seq, data = self.read_header_size(data, self.__from_client)

        data_len = len(data)
        remaining_data = self._size - len(self.__raw)
        data_read = remaining_data if remaining_data <= data_len else data_len

        self.__raw += data[:data_read]

        self._complete = len(self.__raw) == self._size

        if self._complete:
            self.__opcode = unpack('!H', self.__raw[:2])[0]
            if self.__opcode == 0x3c03 and self.__from_client:
                self.__data = community_decode(self.__raw[2:], seq % 20)
            else:
                self.__data = self.__raw[2:]
            del self.__raw

        return data[data_read:]

    @property
    def from_client(self) -> bool:
        return self.__from_client

    @property
    def complete(self) -> bool:
        return self._complete

    @property
    def seek(self) -> int:
        return self._seek

    @seek.setter
    def seek(self, v):
        if v > self._size - 2:
            raise ValueError()

        self._seek = v

    @property
    def size(self) -> int:
        return self._size

    @property
    def data(self) -> bytes:
        return self.__data

    @property
    def opcode(self) -> bytes:
        if not self._complete:
            raise RuntimeError()

        return self.__opcode

    def read_data(self, count: int) -> bytes:
        if self._seek + count > self._size - 2:
            raise IndexError('Not enough data left to read')

        a = self._seek
        b = self._seek = self._seek + count

        return self.__data[a:b]

    def read_byte(self) -> int:
        return self.read_data(1)[0]

    def read_ushort(self) -> int:
        return unpack('!H', self.read_data(2))[0]

    def read_uint(self) -> int:
        return unpack('!I', self.read_data(4))[0]

    def read_ulong(self) -> int:
        return unpack('!L', self.read_data(8))[0]

    def read_string(self) -> str:
        s = self.read_ushort()
        return self.read_data(s).decode()


class DeadMazeClient(ProxyClient):

    MIN_SEQUENCE = 0x00
    MAX_SEQUENCE = 0x63

    def __init__(self, s: socket, addr: tuple) -> None:
        super().__init__(s, addr)
        self._sequence = 0x20

        self._cpacket = DeadMazePacket(True)
        self._spacket = DeadMazePacket()

    def _increment_sequence(self) -> bytes:
        self._sequence += 1
        if self._sequence > self.MAX_SEQUENCE:
            self._sequence = self.MIN_SEQUENCE

        return self._sequence.to_bytes()

    def write(self, p: DeadMazePacket) -> None:
        sfd = self._remote if p.from_client else self._client

        seq = self._increment_sequence() if p.from_client else b''
        op = pack("!H", p.opcode)
        data = p.data if not(p.opcode == 0x3c03 and p.from_client) else community_encode(p.data, self._sequence % 20)

        sfd.send(DeadMazePacket.encode_header_size(p.size) + seq + op + data)

    def process(self, fd: int) -> bool:
        if fd not in self.filesno():
            raise ValueError("invalid fd")

        sin, callback = (self._client, self.data_sent)\
            if fd == self._client.fileno() else\
            (self._remote, self.data_received)

        d = sin.recv(4096)
        callback(d)

        return len(d) != 0

    def client_connected(self, sock_in: Tuple[str, int],
                         sock_to: Tuple[str, int]) -> None:
        print(f"client_connect from {sock_in} to {sock_to}")

    def data_received(self, b: bytes) -> None:
        while b:
            if self._spacket.complete:
                self._spacket = DeadMazePacket()
            b = self._spacket.write(b)

            if self._spacket.complete:
                self.server_packet(self._spacket)
                self.write(self._spacket)

    def data_sent(self, b: bytes) -> None:
        while b:
            if self._cpacket.complete:
                self._cpacket = DeadMazePacket(True)
            b = self._cpacket.write(b)

            if self._cpacket.complete:
                self.client_packet(self._cpacket)
                self.write(self._cpacket)

    def client_packet(self, p: DeadMazePacket) -> None:
        try:
            parser.process_client(p)
        except Exception as e:
            print(f'Parser failed: {e}')

    def server_packet(self, p: DeadMazePacket) -> None:
        try:
            parser.process_server(p)
        except Exception as e:
            print(f'Parser failed: {e}')


def reload_handler(signum, frame):
    print("[+] DeadMaze: Reloading parser module")
    importlib.reload(parser)
