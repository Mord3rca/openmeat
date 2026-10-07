# Key Breaker

This will parse a Deadmaze Client transaction to find the
community key (used for 0x3c03 opcodes)

## Usage

* Record with wireshark a Deadmaze transaction

* Make sure to create [0x3c03](../../doc/protocol/deadmaze/client/3c03.md) packets,
you can do it by clicking on the social button in the bottom left corner (the heart).

* Feed the pcap file to this utility and hope for the best

* Did not work ? Back to step 1.

## Cracking method

When sending a [0x3c03](../../doc/protocol/deadmaze/client/3c03.md), the first short right after the opcode
is a packet ID and since most of them are <255, the first byte this ID expose a byte of the msg key.

However, it only expose the lower byte of each possible entry of the key (which is composed of 20x 32 bits integer)

Also, it does not expose the order of the key (which can be calculated via the seq parameter of the packet)

Since the key is generated from a "seeded" DJB2 hash (from a string + a master key), we can quickly calculate
possible hashes to regenerate it.

## TODO

Break the master key now, so we can calculate identification key.

But it's a key of 640 bits .... Not gonna be that fast.
