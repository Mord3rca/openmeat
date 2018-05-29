#include "callback.hpp"

PrintCallbacks pcallbacks;

void prettyprint(std::ostream& out, const unsigned char* data, size_t datalen, bool pack_out)
{
  enum delimiters_pos
  {
    SIZE=0,
    GUARD=1,
    CMD=2,
    ARGS=3
  }; size_t delimiters[4];
  
  delimiters[delimiters_pos::SIZE]  = 0;
  delimiters[delimiters_pos::GUARD] = ( data[0] == 0x02 ) ? 3 : 2;
  delimiters[delimiters_pos::CMD]   = (pack_out) ? delimiters[delimiters_pos::GUARD]+1 : delimiters[delimiters_pos::GUARD];
  delimiters[delimiters_pos::ARGS]  = delimiters[delimiters_pos::CMD] +2;
  
  size_t c = 0;
  for( size_t i = 0; i < datalen; i++ )
  {
    if(delimiters[c] == i)
    {
      switch( c )
      {
        case 0:
          out << ANSI_COLOR_BLUE;
          break;
        case 1:
          if( !pack_out )
          {
            out << ANSI_COLOR_YELLOW;
            c++;
          }
          else
            out << ANSI_COLOR_RED;
          break;
        case 2:
          out << ANSI_COLOR_YELLOW;
          break;
        case 3:
          out << ANSI_COLOR_GREEN;
          break;
        default:
          out << ANSI_COLOR_RESET;
      }
      c++;
    }
    out << std::setw(2) << std::setfill('0') << std::hex << +data[i] << ' ';
  }
  
  out << ANSI_COLOR_RESET << std::endl;
}

void PrintCallbacks::onConnect( const struct sockaddr_in &client, const struct sockaddr_in &server)
{
  std::cout << "A TCP stream have been hooked: "
            << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port)
            << " -> " << inet_ntoa(server.sin_addr) << ":" << ntohs(server.sin_port)
            << std::endl;
}

void PrintCallbacks::onDisconnect()
{
  std::cout << "The connection have stopped." << std::endl;
}

void PrintCallbacks::onError( const std::string &err)
{
  std::cerr << "Woops, something went wrong:" << std::endl << err << std::endl;
}

void PrintCallbacks::onReceived( const unsigned char* data, size_t len)
{
  std::cout << "[IN] - "; prettyprint(std::cout, data, len);
}

void PrintCallbacks::onSend( const unsigned char* data, size_t len)
{
  std::cout << "[OUT] - "; prettyprint(std::cout, data, len, true);
}

Deadmaze::Network::Callback::Callback() :  _packIn(nullptr), _packOut(nullptr),
                                          _wIn(nullptr), _wOut(nullptr)
{}

Deadmaze::Network::Callback::~Callback()
{
  if( _packIn ) delete _packIn;
  if( _packOut ) delete _packOut;
  
  if( _wIn ) delete _wIn;
  if( _wOut ) delete _wOut;
}

void Deadmaze::Network::Callback::onConnect( const struct sockaddr_in& client, const struct sockaddr_in& server)
{
  std::cout << "Connected" << std::endl;
}

void Deadmaze::Network::Callback::onDisconnect( void )
{
  std::cout << "DM Connection closed..." << std::endl;
}

void Deadmaze::Network::Callback::onError( const std::string& err)
{
  std::cerr << "Something wrong happend: " << err << std::endl;
}

void Deadmaze::Network::Callback::onReceived( const unsigned char* data, size_t len)
{
  const unsigned char* datastream = data;
  
  while( datastream < (data+len) )
  {
    if(!_packIn)
    {
      size_t packetLen = 0;
      try
      {
        packetLen = Deadmaze::Network::Serial::decodeLen( datastream, Deadmaze::Network::Serial::Server );
      } catch ( std::exception &err )
      { std::cerr << ANSI_COLOR_RED << "[-][IN] " << err.what() << ANSI_COLOR_RESET << std::endl; break; }
      
      _packIn = new Deadmaze::Network::Packet();
      _packIn->setLength(packetLen);
      
      _wIn = new Deadmaze::Network::PacketWriter( _packIn );
    }
    else
    {
      try
      {
        _wIn->append(datastream, 1);
        datastream++;
      }
      catch ( std::exception &err )
      {
        std::cerr << ANSI_COLOR_RED << "[-][IN] " << err.what() << ANSI_COLOR_RESET <<std::endl;
        delete _packIn; //Will be auto-forward by proxy class.
        break;
      }
      
      if( _wIn->seek() == _packIn->getLength() )
      {
        std::cout << "[IN] - "; //_packIn->print( std::cout );
        for( size_t i = 0; i < _packIn->getLength(); i++ )
          std::cout << std::hex << std::setfill('0') << std::setw(2) << +(*_packIn)[i] << ' ';
        std::cout << std::endl;
        _packIn = nullptr;
        delete _wIn;
      }
    }
  }
}

void Deadmaze::Network::Callback::onSend( const unsigned char* data, size_t len)
{
  const unsigned char* datastream = data;
  
  while( datastream < (data+len) )
  {
    if(!_packOut)
    {
      size_t packetLen = 0;
      try
      {
        packetLen = Deadmaze::Network::Serial::decodeLen( datastream, Deadmaze::Network::Serial::Client );
      } catch ( std::exception &err )
      { std::cerr << ANSI_COLOR_RED << "[-][OUT] " << err.what() << ANSI_COLOR_RESET << std::endl; break; }
      
      _packOut = new Deadmaze::Network::Packet();
      _packOut->setLength(packetLen);
      _wOut = new Deadmaze::Network::PacketWriter( _packOut );
    }
    else
    {
      try
      {
        _wOut->append( datastream, 1);
        datastream++;
      }
      catch ( std::exception &err )
      {
        std::cerr << ANSI_COLOR_RED << "[-][OUT] " << err.what() << ANSI_COLOR_RESET <<std::endl;
        delete _packOut; //Will be auto-forward by proxy class.
        break;
      }
      
      if( _wOut->seek() == _packOut->getLength() )
      {
        std::cout << "[OUT] - ";
        for( size_t i = 0; i < _packOut->getLength(); i++ )
          std::cout << std::hex << std::setfill('0') << std::setw(2) << +(*_packOut)[i] << ' ';
        std::cout << std::endl;
        _packOut = nullptr;
        delete _wOut;
      }
    }
  }
}

Deadmaze::Network::Callback Deadmaze::Network::callbacks;
