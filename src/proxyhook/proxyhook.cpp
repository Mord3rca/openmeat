#include "proxyhook.hpp"

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  if(addr->sa_family == AF_INET)
  {
    char s[INET_ADDRSTRLEN]; int port = ntohs( ((struct sockaddr_in*)addr)->sin_port );
    inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr, s, sizeof(s));
    std::cout << "[PROXY] TCP connection detected: " << s << ":" << port << std::endl;
    if( std::string(s) == "5.196.91.193")
    {
      std::cout << "[PROXY] Rerouting TCP stream from " << s << ":" << port
                                                        << " to localhost:4444" << std::endl;
      struct sockaddr_in new_addr;
        new_addr.sin_family       = AF_INET;
        new_addr.sin_port         = htons(4444);
        new_addr.sin_addr.s_addr  = inet_addr("127.0.0.1");
      int conn = (*real_fun.connect)(sockfd, (struct sockaddr*)&new_addr, sizeof(new_addr));
      /*if( conn >= 0)*/ sendSOCKS4req(sockfd, addr);
      return conn;
    }
  }
  return (*real_fun.connect)(sockfd, addr, addrlen);
}

void sendSOCKS4req(int sockfd, const struct sockaddr *addr)
{
  const struct sockaddr_in* addr_in = (const struct sockaddr_in*)addr;
  unsigned char socks4req[8];
  socks4req[0] = 4;
  socks4req[1] = 1;
  socks4req[2] = (unsigned char)((addr_in->sin_port >> 8) & 0xff);
  socks4req[3] = (unsigned char)(addr_in->sin_port & 0xff);
  socks4req[4] = ((unsigned char*)&addr_in->sin_addr.s_addr)[0];
  socks4req[5] = ((unsigned char*)&addr_in->sin_addr.s_addr)[1];
  socks4req[6] = ((unsigned char*)&addr_in->sin_addr.s_addr)[2];
  socks4req[7] = ((unsigned char*)&addr_in->sin_addr.s_addr)[3];
  
  std::cout << "[PROXY] Sending minimalist SOCKS4 paquet..." << std::endl;
  send(sockfd, &socks4req, sizeof(socks4req), 0);
}
