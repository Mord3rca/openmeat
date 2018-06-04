#include <iostream>
#include <fstream>
#include <vector>
#include <map>

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
  
  std::map<uint16_t, uint> op_in, op_out;
  while( (*proxy)() )
  {
    while( Deadmaze::Network::callbacks.in.dataAvailable() )
    {
      Deadmaze::Network::Packet *pack; Deadmaze::Network::callbacks.in >> pack;
      
      Deadmaze::Network::PacketReader pack_read(pack);
      uint16_t opcode; pack_read >> opcode;
      
      op_in[opcode]++;
      
      std::cout << "[IN] - "; pack->print( std::cout );
      
      delete pack;
    }
    
    while( Deadmaze::Network::callbacks.out.dataAvailable() )
    {
      Deadmaze::Network::Packet *pack; Deadmaze::Network::callbacks.out >> pack;
      
      Deadmaze::Network::PacketReader pack_read(pack);
      uint16_t opcode; pack_read >> opcode;
      op_out[opcode]++;
      
      std::cout << "[OUT] - "; pack->print( std::cout );
      
      delete pack;
    }
    
    std::this_thread::sleep_for( std::chrono::milliseconds(100) );
  }
  
  proxy->wait();
  if(proxy) delete proxy;
  
  std::fstream fileOut("/tmp/opcode_freq_out", std::fstream::out);
  for( auto& i : op_out )
    fileOut << std::hex << std::setw(4) << std::setfill('0') << std::get<0>(i)
            << " : " << std::dec << std::get<1>(i) << std::endl;
  
  std::fstream fileIn("/tmp/opcode_freq_in", std::fstream::out);
  for( auto& i : op_in )
    fileIn << std::hex << std::setw(4) << std::setfill('0') << std::get<0>(i)
            << " : " << std::dec << std::get<1>(i) << std::endl;
  
  fileOut.close();
  fileIn.close();
  
  return 0;
}
