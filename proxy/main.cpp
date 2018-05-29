#include <iostream>

#include "opcodes.hpp"
#include "proxy.hpp"
#include "callback.hpp"

Proxy* createProxy( void )
{
  Proxy* proxy = new Proxy("127.0.0.1", 4444);
  proxy->callbacks(&Deadmaze::Network::callbacks);
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
