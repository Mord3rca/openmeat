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

    def process_failure(self, fd: int, server) -> None:
        for i in self.filesno():
            server.unregister(i)
        del self

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

        self._p = select.poll()
        self._sock = None
        self._sock_addr = (addr, port)

    def __del__(self):
        del self._p

        if self._sock:
            self._sock.close()
            del self._sock

    def stop(self) -> None:
        self._run = False

    def register(
        self, fd: int, callback: callable,
        callback_fail: callable = None
    ) -> None:
        if fd in self._clients.keys():
            raise ValueError(f"{fd} already register")

        self._p.register(fd, select.POLLIN)
        self._clients[fd] = (callback, callback_fail)

    def unregister(self, fd: int) -> None:
        self._p.unregister(fd)
        del self._clients[fd]

    def _accept_callback(self, fd: int) -> None:
        c = self._client_obj(*self._sock.accept())
        for i in c.filesno():
            self.register(i, c.process, c.process_failure)

    def serve_forever(self) -> None:
        self._run = True
        self._sock = socket.create_server(self._sock_addr, reuse_port=True)
        self.register(self._sock.fileno(), self._accept_callback)

        while self._run:
            try:
                fdEvent = self._p.poll(200)
            except InterruptedError:
                continue

            for fd, event in fdEvent:
                c, cf = self._clients[fd]
                if cf is None:
                    cf = lambda a, b: None  # noqa: E731

                try:
                    if not c(fd):
                        cf(fd, self)
                except Exception:
                    cf(fd, self)
