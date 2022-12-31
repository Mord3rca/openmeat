# OpenMeat - A reversing of deadmaze protocol

## Current goal
I'm currently using the proxy to list and decode all messages in order to implement my own server

## Building

This project is using [CMake](https://cmake.org/)

You can compile the software with:

```
$ mkdir build/
$ cmake [-DCMAKE_BUILD_TYPE=Release] ..
$ make
# make install
```

Some options can be set with `-D`:

| name | default | comment |
|------|---------|---------|
| `CMAKE_BUILD_TYPE` | Release | see [this](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html) for more info |
| `BUILD_TESTS` | Off | Build CppUnit tests |
