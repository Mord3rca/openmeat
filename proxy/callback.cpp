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

Deadmaze::Network::Callback::Callback() : in(0, Serial::Server), out(0, Serial::Client)
{}
Deadmaze::Network::Callback::~Callback(){}

void Deadmaze::Network::Callback::onConnect( const struct sockaddr_in &in_info, const struct sockaddr_in &out_info)
{
  std::cout << "Proxy connected..." << std::endl;
}

void Deadmaze::Network::Callback::onDisconnect()
{
  std::cout << "End of connexion." << std::endl;
}

void Deadmaze::Network::Callback::onError( const std::string &err)
{
  std::cerr << ANSI_COLOR_RED << err << ANSI_COLOR_RESET << std::endl;
}

void Deadmaze::Network::Callback::onSend( const unsigned char *data, size_t len)
{
  out.processIncommingData( data, len );
}

void Deadmaze::Network::Callback::onReceived( const unsigned char *data, size_t len )
{
  in.processIncommingData( data, len );
}

Deadmaze::Network::Callback Deadmaze::Network::callbacks;
