#ifndef HOOK_HPP
#define HOOK_HPP

#include <iostream>
#include <cstring>
#include <array>

extern "C"
{
  #include <dlfcn.h>
  #include <errno.h>

  #include <sys/types.h> //SOCKS struct
  #include <sys/socket.h> // SOCKS
  #include <arpa/inet.h>
}

extern char **environ;

//Extracted from deadmeat.swf
static std::array<in_addr_t, 4> dm_ips =  {{
                                            inet_addr("188.165.233.157"),
                                            inet_addr("164.132.202.12"),
                                            inet_addr("195.154.124.97"),
                                            inet_addr("5.196.91.193")
                                          }};
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

bool isDMGameServer(in_addr_t);
bool SOCKS4negociation(int, const struct sockaddr*, socklen_t);

/*
 * Normally, AS3 can't call getenv(), but just in case:
 * Hide the environment var at library loading.
 * 
 * removeFromEnviron should be in init section of the ELF
 */
void removeFromEnviron( void ) __attribute__(( constructor ));

#endif //HOOK_HPP
