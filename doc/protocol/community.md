# Community commands

Community commands are listed in [0x3c03](opcodes/3c03.md) opcode documentation.

## Encryption

All community commands are obfuscated by a XOR "encryption"

Community commands are **NOT REQUIRED** to play the game. Also, a wrong encryption **DOES NOT** crash the client/server communication.

Only packet payload is encrypted, opcode is still sent in clear.


The start of the key used to encrypt  is defined by the sequence, by the following operation: `sequence % 20`

## Decryption

Exactly as encryption, XOR is a syncronous algorithm.
