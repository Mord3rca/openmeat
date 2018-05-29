#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <iostream>
#include <iomanip>

#include "proxy.hpp"
#include "packet.hpp"
#include "serial.hpp"

extern "C"
{
  #include <arpa/inet.h> //ntohs
}

//https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void prettyprint(std::ostream&, const unsigned char*, size_t, bool = false);

class PrintCallbacks : public Proxy::Callbacks
{
public:
  PrintCallbacks(){}
  ~PrintCallbacks(){}
  
  void onConnect(const struct sockaddr_in&, const struct sockaddr_in&);
  void onDisconnect( void );
  
  void onReceived(const unsigned char*, size_t);
  void onSend(const unsigned char*, size_t);
  
  void onError(const std::string&);
};
extern PrintCallbacks pcallbacks;

namespace Deadmaze::Network
{
  class Callback : public Proxy::Callbacks
  {
  public:
    Callback();
    ~Callback();
    
    void onConnect( const struct sockaddr_in&, const struct sockaddr_in& );
    void onDisconnect( void );
    
    void onError( const std::string& );
    
    void onSend( const unsigned char*, size_t);
    void onReceived( const unsigned char*, size_t);
    
  private:
    Deadmaze::Network::Packet *_packIn, *_packOut;
    Deadmaze::Network::PacketWriter *_wIn, *_wOut;
  };
  extern Callback callbacks;
}
#endif //CALLBACK_HPP
