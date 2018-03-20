#ifndef HOOK_HPP
#define HOOK_HPP

#include <iostream>
#include <string>

extern "C"
{
  #include <dlfcn.h>
  #include <errno.h>

  #include <sys/types.h> //SOCKS struct
  #include <sys/socket.h> // SOCKS
  #include <arpa/inet.h>
}

const std::string dm_ip = "5.196.91.193";

static unsigned char globalTry = 0;

static struct
{
  int (*connect)(int, const struct sockaddr*, socklen_t) =
        (int (*)(int, const struct sockaddr*, socklen_t)) dlsym(RTLD_NEXT, "connect");
} real_fun; //Get it ? Cauz it's fun !

//Functions to hook (force C linking ... Just in case)
extern "C"
{
  int connect(int, const struct sockaddr*, socklen_t);
}

bool SOCKS4negociation(int, const struct sockaddr*, socklen_t);

#endif //HOOK_HPP
