# Key Breaker

This will parse a Deadmaze Client transaction to find the
community key (used for [0x3c03](../../doc/protocol/deadmaze/client/3c03.md) opcode)
& identification key (used for [0x1a08](../../doc/protocol/deadmaze/client/1a08.md) opcode)

## Usage

* Record with wireshark a Deadmaze transaction at the start of DeadMaze app

* Login to the game (to create a [0x1a08](../../doc/protocol/deadmaze/client/1a08.md))

* Make sure to create [0x3c03](../../doc/protocol/deadmaze/client/3c03.md) packets,
you can do it by clicking on the social button in the bottom left corner (the heart).

* Feed the pcap file to this utility and hope for the best

* Did not work ? Back to step 1.

## Cracking method

### MSG Key

When sending a [0x3c03](../../doc/protocol/deadmaze/client/3c03.md), the 2nd data right after the opcode
is a sequence ID (int32). It start from 1 and keep incrementing for each packet so it's value is known.

Each sequence ID is XORed with itself to reveal part of the key (only the lower bytes of each elements).
Since the key is generated from a "seeded" DJB2 hash (from a string + a master key), we can quickly calculate
possible hashes to regenerate it completely.

### IDENTIFICATION Key

Since scramble/unscramble algorithm is now know, we can try all possible keys and unscramble the LOGIN packet until `key_breaker`
find a solution where the size of the packet & it's data type make sense.

TODO: Speedup verif. algorithm by providing character name ?

### ROOT key

Assumption: ROOT Key is generated from the same hash to key algorithm and is not purely RANDOM()

With MSG & IDENTIFICATION Key, it is possible to find a solution where a key can generate a djb2 hash
to match both key generation.

This path is used to speed up IDENTIFICATION crack since testing key derivation is faster than the unscramble algorithm...
But it require a solution for the MSG Key.
