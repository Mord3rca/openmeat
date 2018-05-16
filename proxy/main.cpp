#include <iostream>

#include "opcodes.hpp"
#include "proxy.hpp"
#include "callback.hpp"

inline size_t decodelen(unsigned char* packet)
{
  size_t result = 0;
  for(size_t i = packet[0]; i > 0; i--)
    result |= packet[i] << (packet[0] - i) * 8;
    
  return result;
}

Proxy* createProxy( void )
{
  Proxy* proxy = new Proxy("127.0.0.1", 4444);
  proxy->callbacks(&pcallbacks);
  return proxy;
}

int main( int argc, char *argv[] )
{
  std::cout << "Proxy init, listening to 127.0.0.1:4444" << std::endl;
  Proxy* proxy = createProxy();
  std::cout << "Init complete, waiting..." << std::endl;
  proxy->wait();
  if(proxy) delete proxy;
  
  return 0;
}
