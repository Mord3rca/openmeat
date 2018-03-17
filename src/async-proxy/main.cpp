#include <iostream>
#include <iomanip>

extern "C"
{
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <arpa/inet.h>
  
  #include <unistd.h>
  #include <string.h>
  
  #include <poll.h>
}

void prettyprint(std::ostream&, unsigned char*, size_t, bool = false);

int main( void )
{
  int proxy_sock = socket(AF_INET, SOCK_STREAM, 0);
  if( proxy_sock < 0)
  {
    std::cerr << "socket() error" << std::endl;
    return -1;
  }
  
  struct sockaddr_in serv_bind_addr;
    serv_bind_addr.sin_port         = htons(4444);
    serv_bind_addr.sin_family       = AF_INET;
    serv_bind_addr.sin_addr.s_addr  = inet_addr("127.0.0.1");
  
  if( bind(proxy_sock, (struct sockaddr*)&serv_bind_addr, sizeof(serv_bind_addr)) < 0)
  {
    std::cerr << "bind() Error" << std::endl;
    return -1;
  }
  
  std::cout << "Socket Bind: OK" << std::endl;
  
  if(listen(proxy_sock, 1) < 0)
  {
    std::cerr << "listen() error" << std::endl;
    return -1;
  }
  
  int client_sock = accept(proxy_sock, nullptr, nullptr);
  if(client_sock < 0)
  {
    std::cerr << "accept() error" << std::endl;
    return -1;
  }
  
  char connect_msg[8];
  int sckslen = recv(client_sock, &connect_msg, sizeof(connect_msg), 0);
  if( sckslen < 8)
  {
    std::cerr << "SOCKS4 Received error (len="<< sckslen << ")" << std::endl;
    close(client_sock);
    close(proxy_sock);
    return -1;
  }
  int server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if( server_sock < 0 )
  {
    std::cerr << "Server socket() error" << std::endl;
    close(client_sock);
    return -1;
  }
  
  std::cout << "DM Connection spoofed. Closing proxy listener." << std::endl;
  close(proxy_sock); proxy_sock = 0;
  
  struct sockaddr_in serv_addr; memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    memcpy(&serv_addr.sin_port, &connect_msg[2], 2);
    serv_addr.sin_port = htons(serv_addr.sin_port);
    memcpy(&serv_addr.sin_addr.s_addr, &connect_msg[4], 4);
  
  std::cout << "Connecting to: " << inet_ntoa( serv_addr.sin_addr ) << ":" << serv_addr.sin_port << std::endl;
  
  if( connect(server_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
  {
    std::cerr << "Server connect() error" << std::endl;
    close(client_sock);
    return -1;
  }
  
  std::cout << "Client accepted" << std::endl;
  struct pollfd fds[2];
    fds[0].fd = client_sock;
    fds[0].events = POLLIN;
    fds[1].fd = server_sock;
    fds[1].events = POLLIN;
  
  bool run = true;
  while(run)
  {
    //std::cout << "Waiting on Poll..." << std::endl;
    int ret = poll(fds, 2, 500);
    
    if(ret < 0) break;
    if( ret > 0 )
    {
      unsigned char buff[2048];
        if( fds[0].revents & POLLIN )
        {
          int pos = recv(fds[0].fd, &buff, sizeof(buff), 0);
          if( pos > 0 )
          {
            std::cout << "[OUT] - ";
            //std::cout.write(buff, pos); std::cout << std::endl;
            prettyprint(std::cout, buff, pos, true);
            send(fds[1].fd, &buff, pos, 0);
          }
          else
          {
            std::cout << "End of stream" << std::endl;
            run = false; break;
          }
        }
        
        if( fds[1].revents & POLLIN )
        {
          int pos = recv(fds[1].fd, &buff, sizeof(buff), 0);
          if( pos > 0 )
          {
            std::cout << "[IN] - ";
            //std::cout.write(buff, pos); std::cout << std::endl;
            prettyprint(std::cout, buff, pos);
            send(fds[0].fd, &buff, pos, 0);
          }
          else
          {
            std::cout << "End of stream" << std::endl;
            run = false; break;
          }
        }
        
        if( fds[0].revents & POLLHUP || fds[1].revents & POLLHUP)
        {
          std::cout << "Someone broke the connection" << std::endl;
          run = false; break;
        }
    }
  }
  
  close(client_sock);
  close(server_sock);
  if(proxy_sock > 0) close(proxy_sock);
  return 0;
}

//https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void prettyprint(std::ostream& out, unsigned char* data, size_t datalen, bool pack_out)
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
          if( delimiters[delimiters_pos::GUARD] == delimiters[delimiters_pos::CMD] )
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
