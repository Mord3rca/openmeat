#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#include <dlfcn.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static bool _init = false;
static bool _verbose = false;

static char *hook_ip = "127.0.0.1";
static uint16_t hook_port = 4444;

static int (*real_connect)(int, const struct sockaddr*, socklen_t) = NULL;

static void hook_info(const char *fmt, ...) {
    va_list ap;

    if (!_verbose)
        return;

    va_start(ap, fmt);
    fputs("[HOOK] ", stderr);
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
}

static void hook_init() {
    char *v;
    real_connect = dlsym(RTLD_NEXT, "connect");

    v = getenv("HOOK_ADDR");
    if (v)
        hook_ip = v;

    v = getenv("HOOK_PORT");
    if (v)
        hook_port = atoi(v);

    _verbose = getenv("HOOK_VERBOSE") != NULL;
    hook_info("Ready");
    hook_info("Will redirect DeadMaze connection trough %s:%u",
              hook_ip, hook_port);

    _init = true;
}

static bool is_hooking_required(const struct sockaddr_in *addr) {
    in_port_t port;

    port = ntohs(addr->sin_port);

    hook_info("Inspecting connection %s:%u...",
              inet_ntoa(addr->sin_addr), port);

    return port % 1000 == 801;
}

static bool SOCKS4neg(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    unsigned char socks4req[8] = {0};
    const struct sockaddr_in* addr_in = (const struct sockaddr_in*)addr;
    // Connect to localhost SOCKS4 Proxy
    struct sockaddr_in newaddr;
        newaddr.sin_port            = htons(hook_port);
        newaddr.sin_addr.s_addr     = inet_addr(hook_ip);
        newaddr.sin_family          = AF_INET;

    if (real_connect(sockfd, (const struct sockaddr*)&newaddr, sizeof( newaddr )) != 0) {
        hook_info("  -> could not connect to SOCKS proxy (%s:%u)",
                  hook_ip, hook_port);
        return false;
    }

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

    hook_info("  -> Connection hooked !");

    return true;
}

// Function to hook
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    const struct sockaddr_in *addr_in = NULL;

    if (!_init)
        hook_init();

    if (!addr->sa_family == AF_INET)
        return real_connect(sockfd, addr, addrlen);

    addr_in = (const struct sockaddr_in *)addr;

    // Checking if trying to connect to DM server or not
    if (!is_hooking_required(addr_in)) {
        hook_info("  -> Not hooking for this one.");
        return real_connect(sockfd, addr, addrlen);
    }

    // This will probably mess up errno...
    return SOCKS4neg(sockfd, addr, addrlen) ? 0 : -1;
}

