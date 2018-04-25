# OpenMeat - A reversing of deadmaze protocol

## How to use it ?

Start the proxy and right after that, lead the game with the TCP hook

You have to preload the hook with the following command steam launch option:
```
LD_PRELOAD=/path/to/hook.so %command%
```
or, load the hook in your browser:
```
LD_PRELOAD=/path/to/hook.so firefox
```

## Current goal
I'm currently using the proxy to list and decode all messages in order to implement my own server
