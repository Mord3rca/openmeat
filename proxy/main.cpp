#include <iostream>

#include <thread>

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
  
  while( (*proxy)() )
  {
    while( Deadmaze::Network::callbacks.in.dataAvailable() )
    {
      Deadmaze::Network::Packet *pack; Deadmaze::Network::callbacks.in >> pack;
      
      std::cout << "[IN] - "; pack->print( std::cout );
      
      delete pack;
    }
    
    while( Deadmaze::Network::callbacks.out.dataAvailable() )
    {
      Deadmaze::Network::Packet *pack; Deadmaze::Network::callbacks.out >> pack;
      
      std::cout << "[OUT] - ";pack->print( std::cout );
      
      delete pack;
    }
    
    std::this_thread::sleep_for( std::chrono::milliseconds(100) );
  }
  
  proxy->wait();
  if(proxy) delete proxy;
  
  return 0;
}
