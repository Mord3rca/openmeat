#include "hook.hpp"

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  if(addr->sa_family == AF_INET && globalTry < 3)
  {
    //Checking if trying to connect to DM server
    if( ((struct sockaddr_in*)addr)->sin_addr.s_addr == inet_addr( dm_ip.c_str() ) )
    {
      std::cout << "[PROXY] Rerouting TCP stream from DM Server to localhost:4444" << std::endl;
      if( SOCKS4negociation(sockfd, addr, addrlen) )
        return 0;
      else
      {
        globalTry++;
        shutdown(sockfd, SHUT_RDWR);
      }
    }
  }
  return (*real_fun.connect)(sockfd, addr, addrlen);
}

bool SOCKS4negociation(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  //Connect to localhost SOCKS4 Proxy
  struct sockaddr_in newaddr;
    newaddr.sin_port            = htons(4444);
    newaddr.sin_addr.s_addr     = inet_addr("127.0.0.1");
    newaddr.sin_family          = AF_INET;
  int conn = (*real_fun.connect)(sockfd, (const struct sockaddr*)&newaddr, sizeof( newaddr ) );
  /*
  if( conn != 0 && errno != EINPROGRESS)
  {
    std::cerr << "[PROXY] Can't connect to main App ..." << std::endl;
    return false;
  }
  */
  
  //Connected, send SOCKS4 paquet
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
  /*
  char buffer[16];
  if( recv(sockfd, &buffer, 16, 0) < 2 )
  {
    std::cerr << "[PROXY] Main app did not answer ..." << std::endl;
    return false;
  }
  
  if( buffer[1] != 0x5A )
  {
    std::cerr << "[PROXY] Main app can't etablish connection with the DM server !" << std::endl;
    return false;
  }
  */
  
  return true;
}
