#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <iostream>
#include <iomanip>

#include "proxy.hpp"

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
  
  void onConnect(const struct sockaddr_in&, const struct sockaddr_in&);
  void onDisconnect( void );
  
  void onReceived(const unsigned char*, size_t);
  void onSend(const unsigned char*, size_t);
  
  void onError(const std::string&);
};
extern PrintCallbacks pcallbacks;

#endif //CALLBACK_HPP
