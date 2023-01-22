import os

from deadmaze import DeadMazePacket


class Injector:

    DEFAULT_NAME = "/tmp/om-injector-default-name"

    def __init__(self, path: str = None, clients = None):
        self._path = path if path else self.DEFAULT_NAME
        self._fifo = self._open()
        self._clients = clients

    def __del__(self) -> None:
        if self._fifo.fileno() > 0:
            os.unlink(self._path)

    def _open(self) -> int:
        try:
             os.stat(self._path)
        except FileNotFoundError:
             os.mkfifo(self._path, 0o600)

        return open(self._path, 'w+b', 0)

    @property
    def clients(self):
        return self._clients()

    @property
    def fifo(self):
        return self._fifo.fileno()

    def read(self) -> bytes:
        cmd = self._fifo.read(4096)

        return cmd

    def process(self, fd:int) -> bool:
        r = self.read()
        c = self.clients[0]

        try:
            cmd, *arg = r.split(b' ')
            data = bytes.fromhex(arg[0].decode())
        except Exception:
            print(f'Malformed packet: {r.encode()}')
            return True  # Just skip the command

        if cmd == b'from:client':
            c.inject(data, True)
        elif cmd == b'from:server':
            c.inject(data, False)
        else:
            print('Not understood')

        return True

    def failure(self, fd: int, server) -> None:
        server.unregister(fd)
        del self

