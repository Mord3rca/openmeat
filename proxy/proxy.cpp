#include "proxy.hpp"

Proxy::Proxy() :  _loop(nullptr), _run(true),
                  _proxyfd(0), _clientfd(0), _serverfd(0),
                  _callbacks( &nullCallbacks )
{}

Proxy::Proxy( const std::string ip, int port ) : Proxy()
{
  struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
  
  start(addr);
}

Proxy::Proxy( const struct sockaddr_in addr ) : Proxy()
{
  start(addr);
}

Proxy::~Proxy()
{
  _run.store(false);
  wait();
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
    this->callbacks().onError("Socket(): cannot allocate proxyfd");
    _cleanup(); return;
  }
  
  int turnon = 1;
  setsockopt(_proxyfd, SOL_SOCKET, SO_REUSEADDR, &turnon, sizeof(turnon));
  
  if( bind(_proxyfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    this->callbacks().onError("bind(): Error on proxyfd");
    _cleanup(); return;
  }
  
  if(listen(_proxyfd, 1) < 0)
  {
    this->callbacks().onError("listen() Error on proxyfd");
    _cleanup(); return;
  }
  
  _loop = new std::thread( &Proxy::_proxy_loop, this );
}

void Proxy::stop()
{
  _run.store(false);
}

void Proxy::_proxy_loop()
{
  struct sockaddr_in client_addr; socklen_t client_addr_size = sizeof(client_addr);
  _clientfd = accept(_proxyfd, (sockaddr*)&client_addr, &client_addr_size);
  if(_clientfd < 0)
  {
    this->callbacks().onError("accept() Error");
    _cleanup(); return;
  }
  
  char connect_msg[8];
  int sckslen = recv(_clientfd, &connect_msg, sizeof(connect_msg), 0);
  if( sckslen < 8)
  {
    this->callbacks().onError("SOCKS4 Error");
    _cleanup(); return;
  }
  
  _serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if( _serverfd < 0 )
  {
    this->callbacks().onError("Socket(): Server error");
    _cleanup(); return;
  }
  
  close(_proxyfd); _proxyfd = 0;
  
  struct sockaddr_in serv_addr; memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    memcpy(&serv_addr.sin_port, &connect_msg[2], 2);
    serv_addr.sin_port = htons(serv_addr.sin_port);
    memcpy(&serv_addr.sin_addr.s_addr, &connect_msg[4], 4);
  
  if( connect(_serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
  {
    this->callbacks().onError("Server: connect() error");
    _cleanup();
    return;
  }
  
  this->callbacks().onConnect( client_addr, serv_addr );
  
  struct pollfd fds[2];
    fds[0].fd = _clientfd;
    fds[0].events = POLLIN;
    fds[1].fd = _serverfd;
    fds[1].events = POLLIN;
  
  while(_run)
  {
    int ret = poll(fds, 2, 200);
    
    if(ret < 0)
    {
      this->callbacks().onError("Poll(): Unexpected return");
      break;
    }
    
    if( ret > 0 )
    {
      unsigned char buff[2048];
      
      if( fds[0].revents & POLLIN )
      {
        int pos = recv(fds[0].fd, &buff, sizeof(buff), 0);
        if( pos > 0 )
        {
          this->callbacks().onSend(buff, pos);
          send(fds[1].fd, &buff, pos, 0);
        }
        else
          break;
      }
      
      if( fds[1].revents & POLLIN )
      {
        int pos = recv(fds[1].fd, &buff, sizeof(buff), 0);
        if( pos > 0 )
        {
          this->callbacks().onReceived(buff, pos);
          send(fds[0].fd, &buff, pos, 0);
        }
        else
          break;
      }
      
      if( fds[0].revents & POLLHUP || fds[1].revents & POLLHUP) break;
    }
  }
  this->callbacks().onDisconnect();
  _cleanup();
}

void Proxy::callbacks( Proxy::Callbacks *e )
{ _callbacks = e; }

Proxy::Callbacks& Proxy::callbacks()
{ return (*_callbacks); }

void Proxy::wait( void )
{
  if(_loop->joinable()) _loop->join();
}

void Proxy::_cleanup() noexcept
{
  if( _proxyfd > 0 )  {close(_proxyfd); _proxyfd=0;}
  if( _clientfd > 0 ) {close(_clientfd); _clientfd=0;}
  if( _serverfd > 0 ) {close(_serverfd); _serverfd=0;}
  
  _run.store(false);
}
