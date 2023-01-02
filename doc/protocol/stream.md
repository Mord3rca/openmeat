# Deadmaze Stream

## Structure

The structure is quite simple and follow this scheme:

```
<SIZE>[SEQUENCE]<PACKET><SIZE>[SEQUENCE]<PACKET>....
```

Where:

* `SIZE` is the packet size, it follow its own encoding

* `SEQUENCE` is a byte only sent from the client which increment from 1
at every packet. Initial value seems random but in need to be contain
in the range [0x00, 0x63]

* `PACKET` is the actual payload with a size defined by the previous `SIZE`


## Size calculation

To avoid sending 0x00 on the network stream, the size is calculated as follow:

```
Read the first byte and add the 7 least significant byte to the size

if the 8th byte is 1, another byte will follow (a 7 bytes shift will be required)
else size calculation is done.
```

## Payload

Payload consists of an opcode and its args.

Opcodes are 2 bytes which is the tiniest packet which can be sent.
