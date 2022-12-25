import select
import socket

from typing import Tuple


class ProxyClient:
    def __init__(self, s: socket, addr: tuple) -> None:
        self._client = s
        self._remote = None

        self._socks4(addr)

    def _socks4(self, addr):
        d = self._client.recv(8)
        port, ip = int.from_bytes(d[2:4], "big"), d[4:]
        ip = socket.inet_ntoa(ip)
        port = socket.ntohs(port)
        self._remote = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._remote.connect((ip, port))

        self.client_connected(self._client.getsockname(),
                              self._remote.getsockname())

    def __del__(self):
        self.client_disconnected()

        self._client.close()
        self._remote.close()

    def filesno(self) -> Tuple[int]:
        return self._client.fileno(), self._remote.fileno()

    def process(self, fd: int) -> bool:
        if fd not in self.filesno():
            raise ValueError("invalid fd")

        sin, sout, callback = (self._client, self._remote, self.data_sent)\
            if fd == self._client.fileno() else\
            (self._remote, self._client, self.data_received)

        d = sin.recv(4096)
        callback(d)
        sout.send(d)

        return len(d) != 0

    def client_connected(self, sock_in: Tuple[str, int],
                         sock_to: Tuple[str, int]) -> None:
        pass

    def client_disconnected(self) -> None:
        pass

    def data_received(self, b: bytes) -> None:
        pass

    def data_sent(self, b: bytes) -> None:
        pass


class ProxyServer:

    def __init__(self, addr: str, port: int, cobj=ProxyClient) -> None:
        self._run = False
        self._client_obj = cobj
        self._clients = {}

        self._sock = (addr, port)

    def stop(self) -> None:
        self._run = False

    def serve_forever(self) -> None:
        self._run = True

        with socket.create_server(self._sock, reuse_port=True) as s:
            p = select.poll()
            p.register(s, select.POLLIN)

            while self._run:
                try:
                    fdEvent = p.poll(200)
                except InterruptedError:
                    continue

                for fd, event in fdEvent:
                    if fd == s.fileno():
                        c = self._client_obj(*s.accept())
                        for i in c.filesno():
                            p.register(i, select.POLLIN)
                            self._clients[i] = c
                    else:
                        c = self._clients[fd]
                        if event & select.POLLIN:
                            if not c.process(fd):
                                for i in c.filesno():
                                    p.unregister(i)
                                    del self._clients[i]
                                del c
