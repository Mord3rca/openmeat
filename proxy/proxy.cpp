#include "proxy.hpp"

Proxy::Proxy() :  _loop(nullptr), _run(true),
                  _proxyfd(0), _clientfd(0), _serverfd(0)
{
  
}

Proxy::Proxy( const std::string ip, int port )
{
  struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
  
  start(addr);
}

Proxy::Proxy( const struct sockaddr_in addr )
{
  start(addr);
}

Proxy::~Proxy()
{
  _run.store(false);
  _loop->join();
  
  close(_proxyfd);
  close(_clientfd);
  close(_serverfd);
}

bool Proxy::operator ()()
{
  return _run;
}

void Proxy::start( const struct sockaddr_in addr )
{
  _proxyfd = socket(AF_INET, SOCK_STREAM, 0);
  if( _proxyfd < 0)
  {
    //std::cerr << "socket() error" << std::endl;
    return;
  }
  
  int turnon = 1;
  setsockopt(_proxyfd, SOL_SOCKET, SO_REUSEADDR, &turnon, sizeof(turnon));
  
  if( bind(_proxyfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    //std::cerr << "bind() Error" << std::endl;
    if(_proxyfd > 0) close(_proxyfd);
    return;
  }
  
  //std::cout << "Socket Bind: OK" << std::endl;
  
  if(listen(_proxyfd, 1) < 0)
  {
    //std::cerr << "listen() error" << std::endl;
    if(_proxyfd > 0) close(_proxyfd);
    return;
  }
  
  _loop = new std::thread( &Proxy::_proxy_loop, this );
}

void Proxy::inject( const std::string cmd )
{
  _inject.push( cmd );
}

void Proxy::_proxy_loop()
{
  _clientfd = accept(_proxyfd, nullptr, nullptr);
  if(_clientfd < 0)
  {
    //std::cerr << "accept() error" << std::endl;
    if(_proxyfd > 0) close(_proxyfd);
    return;
  }
  
  char connect_msg[8];
  int sckslen = recv(_clientfd, &connect_msg, sizeof(connect_msg), 0);
  if( sckslen < 8)
  {
    //std::cerr << "SOCKS4 Received error (len="<< sckslen << ")" << std::endl;
    close(_clientfd);
    if(_proxyfd > 0) close(_proxyfd);
    return;
  }
  
  _serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if( _serverfd < 0 )
  {
    //std::cerr << "Server socket() error" << std::endl;
    close(_serverfd);
    if(_proxyfd > 0) close(_proxyfd);
    return;
  }
  
  //std::cout << "DM Connection spoofed. Closing proxy listener." << std::endl;
  close(_proxyfd); _proxyfd = 0;
  
  struct sockaddr_in serv_addr; memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    memcpy(&serv_addr.sin_port, &connect_msg[2], 2);
    serv_addr.sin_port = htons(serv_addr.sin_port);
    memcpy(&serv_addr.sin_addr.s_addr, &connect_msg[4], 4);
  
  //std::cout << "Connecting to: " << inet_ntoa( serv_addr.sin_addr ) << ":" << serv_addr.sin_port << std::endl;
  
  if( connect(_serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
  {
    //std::cerr << "Server connect() error" << std::endl;
    close(_clientfd);
    return;
  }
  
  //std::cout << "Client accepted" << std::endl;
  struct pollfd fds[2];
    fds[0].fd = _clientfd;
    fds[0].events = POLLIN;
    fds[1].fd = _serverfd;
    fds[1].events = POLLIN;
  
  while(_run)
  {
    //std::cout << "Waiting on Poll..." << std::endl;
    int ret = poll(fds, 2, 200);
    
    if(ret < 0) break;
    
    while(!_inject.empty())
    {
      _sendviaCmdStr(fds[1].fd);
    }
    
    if( ret > 0 )
    {
      unsigned char buff[2048];
      
      if( fds[0].revents & POLLIN )
      {
        int pos = recv(fds[0].fd, &buff, sizeof(buff), 0);
        if( pos > 0 )
        {
          /*
          setGuardValue(buff);
          
          if( printOUT )
          {
            std::cout << "[OUT] - ";
            prettyprint(std::cout, buff, pos, true);
          }
          */
          send(fds[1].fd, &buff, pos, 0);
        }
        else
        {
          //std::cout << "End of stream" << std::endl;
          _run = false; break;
        }
      }
      
      if( fds[1].revents & POLLIN )
      {
        int pos = recv(fds[1].fd, &buff, sizeof(buff), 0);
        if( pos > 0 )
        {
          /*
          if( !isGuardSet && buff[2] == 0x1a && buff[3] == 0x03 )
          {
            current_guard = buff[8];
            isGuardSet = true;
          }
          
          if(printIN)
          {
            std::cout << "[IN] - ";
            prettyprint(std::cout, buff, pos);
          }
          */
          send(fds[0].fd, &buff, pos, 0);
        }
        else
        {
          //std::cout << "End of stream" << std::endl;
          _run = false; break;
        }
      }
      
      if( fds[0].revents & POLLHUP || fds[1].revents & POLLHUP)
      {
        //std::cout << "Someone broke the connection" << std::endl;
        _run = false; break;
      }
    }
  }
}

//ONLY CMD.size() < 252 supported.
void Proxy::_sendviaCmdStr(int fd)
{
  std::string cmd = _inject.front(); _inject.pop();
  unsigned char data[255]; bzero(&data, 255);
  data[0] = 0x01; data[1] = cmd.length()/2;
  //setGuardValue(data);
  
  for(size_t i=0; i<cmd.length(); i++)
  {
    data[i/2 + 3] += ( cmd[i] >= 0x30 && cmd[i] <= 0x39 ) ? (cmd[i] - 0x30)       << ((i%2 -1) * -4):
                                                            (cmd[i] - 0x61+0x0a)  << ((i%2 -1) * -4);
  }
  
  //std::cout << "[INJECTED][OUT] - "; prettyprint(std::cout, data, data[1] + 3, true);
  send(fd, data, data[1] + 3, 0);
}
