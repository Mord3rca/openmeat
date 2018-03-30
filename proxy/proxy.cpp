#include "proxy.hpp"

Proxy::Proxy() :  _loop(nullptr), _run(true),
                  _proxyfd(0), _clientfd(0), _serverfd(0)
{
  _callbacks = {{nullptr, nullptr, nullptr, nullptr, nullptr}};
}

Proxy::Proxy( const std::string ip, int port ) :  _loop(nullptr), _run(true),
                                                  _proxyfd(0), _clientfd(0), _serverfd(0)
{
  struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
  
  start(addr);
}

Proxy::Proxy( const struct sockaddr_in addr ) : _loop(nullptr), _run(true),
                                                _proxyfd(0), _clientfd(0), _serverfd(0)
{
  start(addr);
}

Proxy::~Proxy()
{
  _run.store(false);
  _loop->join();
  
  if(_proxyfd)  close(_proxyfd);
  if(_clientfd) close(_clientfd);
  if(_serverfd) close(_serverfd);
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
    _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"socket() Error on proxyfd", 0);
    _run.store(false); return;
  }
  
  int turnon = 1;
  setsockopt(_proxyfd, SOL_SOCKET, SO_REUSEADDR, &turnon, sizeof(turnon));
  
  if( bind(_proxyfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"bind() Error on proxyfd", 0);
    if(_proxyfd > 0) close(_proxyfd);
    _run.store(false); return;
  }
  
  if(listen(_proxyfd, 1) < 0)
  {
    _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"listen() Error on proxyfd", 0);
    if(_proxyfd > 0) close(_proxyfd);
    _run.store(false); return;
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
    _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"accept() Error", 0);
    if(_proxyfd > 0) close(_proxyfd);
    _run.store(false); return;
  }
  
  char connect_msg[8];
  int sckslen = recv(_clientfd, &connect_msg, sizeof(connect_msg), 0);
  if( sckslen < 8)
  {
    _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"SOCKS4 Error", 0);
    close(_clientfd);
    if(_proxyfd > 0) close(_proxyfd);
    _run.store(false); return;
  }
  
  _serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if( _serverfd < 0 )
  {
    _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"Server Socket() error", 0);
    close(_serverfd);
    if(_proxyfd > 0) close(_proxyfd);
    _run.store(false); return;
  }
  
  close(_proxyfd); _proxyfd = 0;
  
  struct sockaddr_in serv_addr; memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    memcpy(&serv_addr.sin_port, &connect_msg[2], 2);
    serv_addr.sin_port = htons(serv_addr.sin_port);
    memcpy(&serv_addr.sin_addr.s_addr, &connect_msg[4], 4);
  
  //std::cout << "Connecting to: " << inet_ntoa( serv_addr.sin_addr ) << ":" << serv_addr.sin_port << std::endl;
  
  if( connect(_serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
  {
    _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"Server: connect() error", 0);
    close(_clientfd); _run.store(false);
    return;
  }
  
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
          if( _callbacks[ Proxy::CALLBACK_TYPES::ON_SENT ] != nullptr )
            _callbacks[ Proxy::CALLBACK_TYPES::ON_SENT ] (buff, pos);
          
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
          if(_callbacks[ Proxy::CALLBACK_TYPES::ON_RECEIVED ])
            _callbacks[ Proxy::CALLBACK_TYPES::ON_RECEIVED ](buff, pos);
          
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
        _callbacks[Proxy::CALLBACK_TYPES::ON_ERROR]((unsigned char*)"Someone broke the connection", 0);
        _run = false; break;
      }
    }
  }
  
  if(_callbacks[ Proxy::CALLBACK_TYPES::ON_STOP ]) _callbacks[ Proxy::CALLBACK_TYPES::ON_STOP ](nullptr, 0);
  
  close(_clientfd); _clientfd = 0;
  close(_serverfd); _serverfd = 0;
}

void Proxy::setCallback(void (*func)(unsigned char*, size_t), enum CALLBACK_TYPES calltype)
{
  _callbacks[calltype] = func;
}

//ONLY CMD.size() < 252 supported.
void Proxy::_sendviaCmdStr(int fd)
{
  std::string cmd = _inject.front(); _inject.pop();
  unsigned char data[255]; bzero(&data, 255);
  data[0] = 0x01; data[1] = cmd.length()/2;
  
  for(size_t i=0; i<cmd.length(); i++)
  {
    data[i/2 + 3] += ( cmd[i] >= 0x30 && cmd[i] <= 0x39 ) ? (cmd[i] - 0x30)       << ((i%2 -1) * -4):
                                                            (cmd[i] - 0x61+0x0a)  << ((i%2 -1) * -4);
  }
  
  _callbacks[Proxy::CALLBACK_TYPES::ON_INJECT](data, data[1] + 3);
  
  send(fd, data, data[1] + 3, 0);
}
