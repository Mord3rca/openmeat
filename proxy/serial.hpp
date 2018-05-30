#ifndef DM_NET_SERIAL_HPP
#define DM_NET_SERIAL_HPP

#include <mutex>
#include <queue>

extern "C"
{
  #include <sys/types.h>
  #include <sys/socket.h>
  
  #include <unistd.h>
  #include <string.h>
}

#include "packet.hpp"

namespace Deadmaze::Network
{
  class Serial
  {
  public:
    enum TYPE
    {
      Client,
      Server
    };
    
  public:
    Serial( int, TYPE = Client );
    ~Serial();
    
    friend Serial& operator <<( Serial&, const Packet& );
    friend Serial& operator >>( Serial&, Packet*& );
    
    const int getSock() const noexcept;
    const TYPE getType() const noexcept;
    
    static size_t decodeLen( const unsigned char*&, TYPE );
    
    void processIncommingData();
    void processIncommingData( const unsigned char*, const size_t );
    
    bool dataAvailable() noexcept;
    
  private:
    int _sock; TYPE _type;
    unsigned char _GUARD_BYTE;
    
    std::mutex _mtx;
    std::queue< Packet* > _inPacket;
    Packet* _pack;
    PacketWriter* _writer;
  };
}

#endif //DM_NET_SERIAL_HPP
