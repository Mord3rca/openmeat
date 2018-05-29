#ifndef DM_PACKET_HPP
#define DM_PACKET_HPP

#include <iostream>

#include <ostream>
#include <iomanip>

#include <stdexcept>

#include "opcodes.hpp"

extern "C"
{
  #include <sys/types.h>
  #include <sys/socket.h>
  
  #include <unistd.h>
  #include <string.h>
}

namespace Deadmaze::Network
{
  static unsigned char GUARD_BYTE = 0;
  
  class Packet
  {
  public:
    Packet();
    ~Packet();
    
    void print( std::ostream& );
    
    size_t getLength() const;
    void setLength( const size_t );
    
    unsigned char* getRawData() const;
    unsigned char& operator[](size_t) const;
    
  protected:
    unsigned char* _data;
    size_t  _len;
  };
  
  class PacketWriter
  {
  public:
    PacketWriter( Packet& );
    PacketWriter( Packet* );
    
    const size_t seek() const noexcept
    { return _seek; }
    void seek(const size_t s) noexcept
    { _seek = s; }
    
    void reserve( const size_t );
    size_t append( const unsigned char*, const size_t);
    
    void writeString( const std::string& );
    void writeString( const unsigned char*, const size_t );
    
    template<class T>
    void write( const T& );
    
    void operator<<( const std::string& );
    void operator<<( const unsigned char& );
    void operator<<( const uint16_t& );
    void operator<<( const uint32_t& );
    void operator<<( const float& );
    void operator<<( const double& );
    
  private:
    Packet *_pack;
    size_t _seek;
  };
  
  class PacketReader
  {
  public:
    PacketReader( const Packet* );
    PacketReader( const Packet& );
    ~PacketReader();
    
    const size_t seek() const noexcept
    { return _seek; }
    void seek(const size_t s) noexcept
    { _seek = s; }
    
    const std::string readString( void );
    
    template<class T>
    const T read();
    
    void operator>>( std::string& );
    void operator>>( unsigned char& );
    void operator>>( uint16_t& );
    void operator>>( uint32_t& );
    void operator>>( float& );
    void operator>>( double& );
    
  private:
    const Packet *_pack;
    size_t _seek;
  };
}

#endif //DM_PACKET_HPP
