#include "packet.hpp"

Deadmaze::Network::Packet::Packet() : _data(nullptr), _len(0)
{}

Deadmaze::Network::Packet::~Packet()
{
  if( _data ) delete[] _data;
}

void Deadmaze::Network::Packet::setLength( const size_t len )
{
  if( len == 0 )
    throw std::invalid_argument("Packet cannot be of length 0");
  
  unsigned char* new_data = new unsigned char[ len ];
  if( _data )
  {
      memcpy( new_data, _data, ( (len >= _len) ? len : _len ) );
      delete[] _data;
  }
  _data = new_data;
  _len = len;
}

void Deadmaze::Network::Packet::print( std::ostream& out )
{
  out << "Packet Type: RAW PACKET" << std::endl
      << "  Size: " << _len << std::endl
      << "  DATA: ";
  for( size_t i = 0; i < _len; i++ )
    out << std::setw(2) << std::setfill('0') << std::hex << +_data[i] << ' ';
    
  out << std::endl;
}

size_t Deadmaze::Network::Packet::getLength() const
{ return _len; }

unsigned char* Deadmaze::Network::Packet::getRawData() const
{ return _data; }

unsigned char& Deadmaze::Network::Packet::operator []( size_t pos ) const
{
  if( pos > _len )
    throw std::out_of_range("Packet: operator[] out of range.");
  
  return _data[pos];
}

Deadmaze::Network::PacketWriter::PacketWriter( Packet* pack ) : _pack(pack), _seek(0)
{}

Deadmaze::Network::PacketWriter::PacketWriter( Packet& pack ) : _seek(0)
{
  _pack = &pack;
}

void Deadmaze::Network::PacketWriter::reserve( const size_t size)
{
  _pack->setLength( size );
}

size_t Deadmaze::Network::PacketWriter::append( const unsigned char* data, const size_t len )
{
  if( len > ( _pack->getLength() - _seek ) )
    throw std::out_of_range( "PacketWriter: Not enough space for write operation." );
  
  unsigned char *datastart = _pack->getRawData();
  datastart += _seek;
  memcpy( datastart, data, len);
  _seek+=len;
  
  return len;
}

void Deadmaze::Network::PacketWriter::writeString( const std::string& str)
{
  writeString(  reinterpret_cast<const unsigned char*>(str.c_str()),
                str.size() );
}

void Deadmaze::Network::PacketWriter::writeString( const unsigned char* c, const size_t s)
{
  if( s > 255 )
    throw std::out_of_range("PacketWriter: String too long");
  
  write<unsigned char>( s );
  
  for( size_t i = 0; i < s; i++ )
    write<unsigned char>(c[i]);
  
  write<unsigned char>(0);
}

template<class T>
void Deadmaze::Network::PacketWriter::write( const T &e )
{
  if( _pack->getLength() < _seek + sizeof(T) )
    throw std::out_of_range("PacketWriter: Not enough space for write operation.");
  
  union
  {
    T e;
    unsigned char c[ sizeof(T) ];
  } u; u.e = e;
  
  for( size_t i = sizeof(T)-1;; i-- )
  {
    (*_pack)[_seek++] = u.c[ i ];
    if(i == 0) break;
  }
}

void Deadmaze::Network::PacketWriter::operator<<( const std::string& s)
{ writeString(s); }

void Deadmaze::Network::PacketWriter::operator<<( const unsigned char& e)
{ write<unsigned char>(e); }

void Deadmaze::Network::PacketWriter::operator<<( const uint16_t& e)
{ write<uint16_t>(e); }

void Deadmaze::Network::PacketWriter::operator<<( const uint32_t& e)
{ write<uint32_t>(e); }

void Deadmaze::Network::PacketWriter::operator<<( const float& e)
{ write<float>(e); }

void Deadmaze::Network::PacketWriter::operator<<( const double& e)
{ write<double>(e); }

Deadmaze::Network::PacketReader::PacketReader( const Packet* pack) : _pack(pack), _seek(0)
{}

Deadmaze::Network::PacketReader::PacketReader( const Packet& pack) : _seek(0)
{
  _pack = &pack;
}

Deadmaze::Network::PacketReader::~PacketReader(){}

//Only small strings (<= 255 char)
const std::string Deadmaze::Network::PacketReader::readString()
{
  std::string result("");
  //Get the size (first 8 bits of the string)
  size_t size = (*_pack)[_seek++];
  
  //Not enough data left, throw an error
  if( _pack->getLength() < _seek + size )
    throw std::out_of_range("PacketReader: Not enough data left to read a String.");
  
  //Reserve size for the string
  result.reserve( size );
  
  for( size_t i = size; i > 0; i-- )
    result += (*_pack)[_seek++];
  
  _seek++; //String null terminated, but this is ignored here.
  
  return result;
}

template<class T>
const T Deadmaze::Network::PacketReader::read()
{
  if( _pack->getLength() < _seek + sizeof(T) )
    throw std::out_of_range("PacketReader: Not enough data to read");
  
  union
  {
    T e;
    unsigned char c[ sizeof(T) ];
  } u;
  
  for(size_t i = sizeof(T)-1;;i--)
  {
    u.c[ i ] = (*_pack)[_seek++];
    if ( i==0 ) break;
  }
  
  return u.e;
}

void Deadmaze::Network::PacketReader::operator >>( double& s )
{ s = read<double>(); }

void Deadmaze::Network::PacketReader::operator >>( float& s )
{ s = read<float>(); }

void Deadmaze::Network::PacketReader::operator >>( std::string& s )
{ s = readString(); }

void Deadmaze::Network::PacketReader::operator >>( uint16_t& s )
{ s = read<uint16_t>(); }

void Deadmaze::Network::PacketReader::operator >>( uint32_t& s )
{ s = read<uint32_t>(); }

void Deadmaze::Network::PacketReader::operator >>( unsigned char& s )
{ s = read<unsigned char>(); }
