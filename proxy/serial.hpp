#ifndef DM_NET_SERIAL_HPP
#define DM_NET_SERIAL_HPP

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
    friend Serial& operator >>( Serial&, Packet& );
    
    const int getSock() const noexcept;
    const TYPE getType() const noexcept;
    
    static size_t decodeLen( const unsigned char*&, TYPE );
    
  private:
    int _sock; TYPE _type;
  };
}

#endif //DM_NET_SERIAL_HPP
