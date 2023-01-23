# Deadmaze Proxy

## Require

* \>=Python 3.8


## Goal

This is used for reversing DM protocol.

`parser.py` will print client packets by default but can be reloaded by
sending SIGUSR1 to the proxy anytime.


## Usage

Run it with:
```
./run
```

It will create a listening socket on localhost:4444


Use the [hook](../hook) to redirect Deadmaze connection to this proxy.


### Injection

To inject a packet to the data stream, write in the unix FIFO `/tmp/om-injector`

2 commands are supported:

* *from:client* which send the packet to the server

* *from:server* which send the packet to the client

Both command are followed by the packet as HEX STREAM (example: `1A1A` for keep alive)
