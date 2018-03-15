#ifndef PROXY_HOOK_HPP
#define PROXY_HOOK_HPP

#include <iostream>
#include <sstream>
#include <string>

extern "C"
{
  #include <dlfcn.h>

  #include <sys/types.h> //SOCKS struct
  #include <sys/socket.h> // SOCKS
  #include <arpa/inet.h>
}

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

//Should be inline... (g++ will do it for me ... I guess)
void sendSOCKS4req(int, const struct sockaddr*);

#endif //PROXY_HOOK_HPP
