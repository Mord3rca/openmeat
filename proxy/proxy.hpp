#ifndef PROXY_HPP
#define PROXY_HPP

#include <thread>
#include <atomic>

#include <queue>

extern "C"
{
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <arpa/inet.h>
  
  #include <unistd.h>
  #include <string.h>
  
  #include <poll.h>
}

class Proxy
{
public:
  class Callbacks
  {
  public:
    Callbacks(){};
    
    virtual void onReceived( const unsigned char*, size_t ) = 0;
    virtual void onSend( const unsigned char*, size_t ) = 0;
    
    virtual void onConnect( const struct sockaddr_in&, const struct sockaddr_in& ) = 0;
    virtual void onDisconnect( void ) = 0;
    
    virtual void onError( const std::string& ) = 0;
  };
  
  Proxy();
  Proxy( std::string, int );
  Proxy( const struct sockaddr_in );
  
  bool operator()();
  
  void start( const struct sockaddr_in );
  void stop();
  
  void callbacks( Callbacks* );
  Callbacks& callbacks( void );
  
  void wait( void );
  
  const int getSockClient( void ) const {return _clientfd;}
  const int getSockServer( void ) const {return _serverfd;}
  
  ~Proxy();
  
protected:
  void _proxy_loop();
  
private:
  std::thread *_loop;
  std::atomic<bool> _run;
  
  int _proxyfd, _clientfd, _serverfd;
  Callbacks* _callbacks;
  
  void _cleanup() noexcept;
};

class ProxyDefaultCallbacks : public Proxy::Callbacks
{
  public:
  ProxyDefaultCallbacks(){};
  
  void onReceived( const unsigned char*, size_t ){}
  void onSend( const unsigned char*, size_t ){}
  
  void onConnect( const struct sockaddr_in&, const struct sockaddr_in& ){}
  void onDisconnect( void ){}
  
  void onError( const std::string& ){}
};
static ProxyDefaultCallbacks nullCallbacks;

#endif //PROXY_HPP
