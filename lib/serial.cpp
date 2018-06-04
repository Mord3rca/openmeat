#include "serial.hpp"

namespace Deadmaze::Network
{
  Serial::Serial(int sock, TYPE type) : _sock(sock), _type(type), _GUARD_BYTE(0)
  {}
  
  Serial::~Serial()
  {}
  
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
      s._GUARD_BYTE++;
      send(s.getSock(), &s._GUARD_BYTE, 1, MSG_MORE);
      if( s._GUARD_BYTE > 0x63 ) s._GUARD_BYTE -= 0x64;
    }
    send(s.getSock(), psend.getRawData(), psend.getLength(), 0);
    
    return s;
  }
  
  Serial& operator >>( Serial& s, Packet*& preceive )
  {
    std::lock_guard<std::mutex> lck(s._mtx);
    preceive = s._inPacket.front();
    s._inPacket.pop();
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
  
  void Serial::processIncommingData()
  {
    unsigned char buff[2048];
    size_t bufflen = read(_sock, buff, 2048);
    
    processIncommingData(buff, bufflen);
  }
  
  void Serial::processIncommingData( const unsigned char *data, const size_t len )
  {
    const unsigned char *datastream = data;
    
    while( datastream < (data+len) )
    {
      if(!_pack)
      {
        size_t packlen = 0;
        try
        {
          packlen = decodeLen( datastream, _type );
        } catch( std::exception &err )
        {break;}
        
        _pack = new Packet();
        _writer = new PacketWriter( _pack );
        _pack->setLength( packlen );
      }
      else
      {
        try{
        _writer->append( datastream, 1 );
        datastream++;
        } catch(std::exception &err)
        { delete _pack; _pack = nullptr;
          delete _writer; _writer=nullptr;
          break; }
        
        if( _writer->seek() == _pack->getLength() )
        {
          std::lock_guard<std::mutex> lck( _mtx );
          _inPacket.push( _pack ); _pack=nullptr;
          delete _writer;
        }
      }
    }
  }
  
  bool Serial::dataAvailable() noexcept
  {
    std::lock_guard<std::mutex> lck(_mtx);
    return !_inPacket.empty();
  }
}
