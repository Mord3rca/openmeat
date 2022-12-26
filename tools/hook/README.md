# Hook

## Goal

This hook the connect function to hijack the connection to DeadMaze server
and will redirect it to a proxy.

## Usage

Start the proxy and right after that, load the game with the TCP hook

You have to preload the hook with the following command steam launch option:
```
LD_PRELOAD=/path/to/hook.so %command%
```
or, load the hook with flashplayer:
```
LD_PRELOAD=/path/to/hook.so flashplayer https://www.deadmaze.com/alpha/chargeur-deadmeat.swf
```

## Build

To compile this utility use the following command:

`gcc -o hook.so hook.c -fPIC -shared -D_GNU_SOURCE -ldl`

## Environment

* `HOOK_ADDR`: Set proxy address (default: 127.0.0.1)

* `HOOK_PORT`: Set proxy port (default: 4444)

* `HOOK_VERBOSE`: Activate verbosity (output vary following the number of IPv4 connection)
