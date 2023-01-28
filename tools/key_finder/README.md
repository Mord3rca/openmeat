# Key Finder

This will parse a Deadmaze Client transaction to find the
community key (used for 0x3c03 opcodes)

## Usage

* Record with wireshark a Deadmaze transaction

* Make sure to create [0x3c03](../../doc/protocol/opcodes/3c03.md) packets,
you can do it by clicking on the social button in the bottom left corner (the heart)
or by chatting

* Isolate the TCP Stream of the client an save in RAW

* Feed it to this utility and hope for the best

## Cracking method

According to [0x3c03](../../doc/protocol/opcodes/3c03.md), the second arg DATA sent
is the number of packet sent which is easily known.

This exploit a weakness of XORING which allow you to recover the key by xoring the DATA.

```
DATA ^ KEY ^ DATA = KEY
```

In this case, with a sufficent number of 0x3c03 packets, key can be found.
