import socket
import parser
import importlib

from proxy import ProxyClient
from typing import Tuple


class DeadMazePacket:

    def __init__(self, client=False) -> None:
        self.__raw = b''
        self.__from_client = client

        self._size = 0
        self._seek = 0

        self._complete = False

    def __str__(self):
        return f"""\
DeadMaze {'Client' if self.__from_client else 'Server'} Packet
OpCode: {self.opcode.hex()}
Data:   {self.__raw[2:]}"""

    @staticmethod
    def read_header_size(b: bytes, client: bool) -> Tuple[int, bytes]:
        i = 0
        size = 0

        while i < 4:
            size += (b[i] & 0x7F) << (0 if i == 0 else i*7)

            if not b[i] & 0x80:
                break

            i += 1

        i += 1 if not client else 2

        return size, b[i:]

    def write(self, data: bytes) -> bytes:
        if self._size == 0:
            self._size, data = self.read_header_size(data, self.__from_client)

        data_len = len(data)
        remaining_data = self._size - len(self.__raw)
        data_read = remaining_data if remaining_data <= data_len else data_len

        self.__raw += data[:data_read]

        self._complete = len(self.__raw) == self._size

        return data[data_read:]

    @property
    def complete(self) -> bool:
        return self._complete

    @property
    def size(self) -> int:
        return self._size

    @property
    def raw(self) -> bytes:
        return self.__raw

    @property
    def opcode(self) -> bytes:
        if not self._complete:
            raise ValueError()

        return self.__raw[:2]


class DeadMazeClient(ProxyClient):

    def __init__(self, s: socket, addr: tuple) -> None:
        super().__init__(s, addr)

        self._cpacket = DeadMazePacket(True)
        self._spacket = DeadMazePacket()

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

    def data_sent(self, b: bytes) -> None:
        while b:
            if self._cpacket.complete:
                self._cpacket = DeadMazePacket(True)
            b = self._cpacket.write(b)

            if self._spacket.complete:
                self.client_packet(self._cpacket)

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