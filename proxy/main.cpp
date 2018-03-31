#include <iostream>
#include <iomanip>

#include <atomic>

#include "opcodes.hpp"
#include "proxy.hpp"

bool isGuardSet = false;
static unsigned char current_guard = 0;

std::atomic<bool> print_in(true), print_out(true);

//https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void prettyprint(std::ostream& out, const unsigned char* data, size_t datalen, bool pack_out = false)
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

inline void setGuardValue(unsigned char* data)
{
  data[data[0]+1] = current_guard++;
  current_guard -= ( current_guard >= 0x64 ) ? 0x64 : 0x00;
}

inline size_t decodelen(unsigned char* packet)
{
  size_t result = 0;
  for(size_t i = packet[0]; i > 0; i--)
    result |= packet[i] << (packet[0] - i) * 8;
    
  return result;
}

void on_sent(unsigned char* data, size_t len)
{
  //Some packets can be in groups.
  for( size_t i = 0; i < len; )
  {
    size_t pack_len = decodelen( data+i );
    setGuardValue(data + i);
    if( print_out )
    { 
      std::cout << "[OUT] - ";
      prettyprint(std::cout, data+i, pack_len+data[i]+2, true);
    }
    i += pack_len + 2 + data[i];
  }
}

void on_received(unsigned char* data, size_t len)
{
  if(!isGuardSet && data[2] == 0x1a && data[3] == 0x03)
  {
    current_guard = data[8]; isGuardSet = true;
  }
  
  if( print_in )
  {
    std::cout << "[IN] - ";
    prettyprint(std::cout, data, len);
  }
}

void on_error(unsigned char* errmsg, size_t errlen)
{
  std::cerr << errmsg << std::endl;
}

void on_inject(unsigned char* data, size_t len)
{
  setGuardValue(data);
  std::cout << "[INJECTED][OUT] - "; prettyprint(std::cout, data, len, true);
}

Proxy* createProxy( void )
{
  Proxy* proxy = new Proxy("127.0.0.1", 4444);
    proxy->setCallback(on_sent, Proxy::CALLBACK_TYPES::ON_SENT);
    proxy->setCallback(on_error, Proxy::CALLBACK_TYPES::ON_ERROR);
    proxy->setCallback(on_inject, Proxy::CALLBACK_TYPES::ON_INJECT);
    proxy->setCallback(on_received, Proxy::CALLBACK_TYPES::ON_RECEIVED);
  
  return proxy;
}

int main( int argc, char *argv[] )
{
  Proxy* proxy = createProxy();
  while(true)
  {
    std::string cmd;
    std::cout << "--> "; std::getline(std::cin, cmd);
    if( cmd == "exit" ) break;
    
    if( !cmd.empty() )
    {
      if( cmd.find("print") != std::string::npos )
      {
        if(cmd.find("IN") != std::string::npos)
        {
          std::cout << "Toggle IN Print" << std::endl;
          print_in.store( !print_in );
        }
        else if ( cmd.find("OUT") != std::string::npos )
        {
          std::cout << "Toogle OUT Print" << std::endl;
          print_out.store( !print_out );
        }
        else
          std::cout << "print command require an argument (IN | OUT)" << std::endl;
      }
      else if(cmd.find("status")!=std::string::npos)
        std::cout << "Proxy: " << ( ((*proxy)()) ? "Running" : "Stopped" ) << std::endl;
      else if(cmd.find("restart")!=std::string::npos)
      {
        delete proxy;
        proxy = createProxy();
        std::cout << "Proxy restarted" << std::endl;
        //Reseting guard byte
        isGuardSet = false; current_guard = 0;
      }
      else if( cmd.find("inject") != std::string::npos ) proxy->inject(cmd.substr(7));
      else std::cout << "Command not recognized" << std::endl;
    }
  }
  
  if(proxy) delete proxy;
  
  return 0;
}

/*
#include <FL/Fl.H>
#include "ui/mainwin.hpp"

int main(int argc, char *argv[])
{
  Fl::scheme("plastic");
  ui::mainwin win;
  win.resizable(win);
  win.show(argc, argv);
  return Fl::run();
}
*/
