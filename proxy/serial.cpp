#include "serial.hpp"

namespace Deadmaze::Network
{
  Serial::Serial(int sock, TYPE type) : _sock(sock), _type(type)
  {}
  
  Serial::~Serial(){}
  
  const int Serial::getSock() const noexcept
  { return _sock; }
  
  const Serial::TYPE Serial::getType() const noexcept
  { return _type; }
  
  Serial& operator <<( Serial& s, const Packet& psend )
  {
    //Computing header length.
    unsigned char headerLen = (psend.getLength() >> 8) + 1;
    
    //Writing header
    unsigned char header[ headerLen + 1 ];
    
    header[0] = headerLen;
    union
    {
      size_t i = 0;
      unsigned char s[ sizeof(size_t) ];
    } u;
    u.i = psend.getLength();
    
    for(size_t i = 0; i < headerLen; i++)
      header[i+1] = u.s[ sizeof(size_t) - i];
    
    send(s.getSock(), header, headerLen+1, MSG_MORE);
    if( s.getType() == Serial::Client )
    {
      GUARD_BYTE++;
      send(s.getSock(), &GUARD_BYTE, 1, MSG_MORE);
      if( GUARD_BYTE > 0x63 ) GUARD_BYTE -= 0x64;
    }
    send(s.getSock(), psend.getRawData(), psend.getLength(), 0);
    
    return s;
  }
  
  Serial& operator >>( Serial& s, Packet& preceive )
  {
    return s;
  }
  
  size_t Serial::decodeLen( const unsigned char*& datastream, TYPE type )
  {
    union
    {
      size_t s;
      unsigned char c[sizeof(size_t)];
    } u;
    size_t i = *datastream; u.s = 0;
    
    if( i > sizeof(size_t))
      throw std::out_of_range( std::string(__FUNCTION__) + ": Can't parse size_t, size exceed 64 bits.");
    
    for(; i > 0; i--)
    {
      datastream++; u.c[ i-1 ] = *datastream;
    }
    datastream++;
    if( type == Client ) datastream++;
    
    return u.s;
  }
}
