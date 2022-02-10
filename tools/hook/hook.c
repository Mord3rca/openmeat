#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <dlfcn.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static bool _init = false;
static bool _verbose = false;
static unsigned char globalTry = 0;

static char *hook_ip = "127.0.0.1";
static unsigned short hook_port = 4444;

static int (*real_connect)(int, const struct sockaddr*, socklen_t) = NULL;

static void hook_init()
{
    char *v;
    real_connect = dlsym(RTLD_NEXT, "connect");

    v = getenv("HOOK_ADDR");
    if(v)
        hook_ip = v;

    v = getenv("HOOK_PORT");
    if(v)
        hook_port = atoi(v);

    _verbose = getenv("HOOK_VERBOSE") != NULL;
}

static bool is_hooking_required( const struct sockaddr *addr )
{
    return ntohs(((const struct sockaddr_in*)addr)->sin_port) % 1000 == 801;
}

static bool SOCKS4neg(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    unsigned char socks4req[8] = {0};
    const struct sockaddr_in* addr_in = (const struct sockaddr_in*)addr;
    // Connect to localhost SOCKS4 Proxy
    struct sockaddr_in newaddr;
        newaddr.sin_port            = htons(hook_port);
        newaddr.sin_addr.s_addr     = inet_addr(hook_ip);
        newaddr.sin_family          = AF_INET;
    real_connect(sockfd, (const struct sockaddr*)&newaddr, sizeof( newaddr ));

    // Connected, send SOCKS4 paquet
    socks4req[0] = 4;
    socks4req[1] = 1;
    socks4req[2] = (unsigned char)((addr_in->sin_port >> 8) & 0xff);
    socks4req[3] = (unsigned char)(addr_in->sin_port & 0xff);
    socks4req[4] = ((unsigned char*)&addr_in->sin_addr.s_addr)[0];
    socks4req[5] = ((unsigned char*)&addr_in->sin_addr.s_addr)[1];
    socks4req[6] = ((unsigned char*)&addr_in->sin_addr.s_addr)[2];
    socks4req[7] = ((unsigned char*)&addr_in->sin_addr.s_addr)[3];

    send(sockfd, &socks4req, sizeof(socks4req), 0);

    return true;
}

// Function to hook
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if(!_init)
        hook_init();

    if( ! addr->sa_family == AF_INET )
        return real_connect(sockfd, addr, addrlen);

    //Checking if trying to connect to DM server
    if( is_hooking_required(addr) && globalTry < 3 ) {
        if( SOCKS4neg(sockfd, addr, addrlen) )
            return 0;
        else {
            globalTry++;
            shutdown(sockfd, SHUT_RDWR);
        }
    }

    return real_connect(sockfd, addr, addrlen);
}

