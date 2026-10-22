# Entity2

Defined by §\x04\x02\x04\x01\x03\x05\x04\b\x01\b\x05\x05\x02§.§\x06\x06\x03\x01\b\b\x07\x02\x01\x06\x01\x06\x07§

| name | type | comment |
|------|------|---------|
| | ushort | |
| cmd | ushort | |


## if cmd == 0

| name | type | comment |
|------|------|---------|
| len | ubyte | |
| | <ushort, ushort, ushort, ushort>[len] | 2 last ushort are / 100|


## else

Seems to manage something related to color (alpha / couleur)

| name | type | comment |
|------|------|---------|
| | byte | |
| | byte | |
| | short | / 100 |
| | short | / 100 |
| | ushort | / 100 |
| | ushort | / 100 |
| | ubyte | |
| | Boolean | |
| | Boolean | |
| | int | if last Bool is true|
