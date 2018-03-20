# DeadMeat Cheat Engine
A TCP Hook to inject and visualise packets received/sent by Dead Maze

## How to use it ?

You have to preload the hook with the following command steam launch option:
```
LD_PRELOAD=/path/to/hook.so %command%
```

Once done, start the proxy and start the game.

## Current goal
Actually, I'm focusing on decoding packets opcode. Once it's done, I'll search for some vulnerability in the communication protocol.

