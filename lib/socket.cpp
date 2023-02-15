#include "openmeat/socket"

#include <cstring>

extern "C"
{
    #include <sys/types.h>
    #include <sys/socket.h>

    #include <unistd.h>
}

using namespace Openmeat::Network;

Socket::Socket(TYPE type) : __sock(-1), __type(type), __sequence(0),
                            __in_buff(nullptr), __plen(0), __packet(nullptr)
{}

Socket::Socket(int fd, TYPE type) : Socket(type) {
    __sock = fd;
}

Socket::~Socket() {
    if(__sock != -1) close(__sock);

    if(__in_buff) delete[] __in_buff;

    if(__packet) delete __packet;

    while(!__packets.empty()) {
        auto p = __packets.front();
        __packets.pop();
        delete p;
    }
}

int Socket::fd() const noexcept
{ return __sock; }

Socket::TYPE Socket::type() const noexcept
{ return __type; }

bool Socket::empty() const noexcept {
    return __packets.empty();
}

unsigned char Socket::sequence() const noexcept {
    return __sequence;
}

bool Socket::connect(const struct sockaddr_in& addr) {
    __sock = socket(AF_INET, SOCK_STREAM, 0);

    if( ::connect(__sock, (const struct sockaddr*)&addr, sizeof(addr)) == -1 ) {
        __sock = -1;
        return false;
    }

    return true;
}

bool Socket::connect(const std::string& ip, const unsigned short port) {
    struct sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());

    return connect(addr);
}

void Socket::read() {
    if(!__in_buff) __in_buff = new unsigned char[IN_BUFF_LEN];
    ssize_t len = recv(__sock, __in_buff, IN_BUFF_LEN, 0);

    read(__in_buff, len);
}

void Socket::read(const unsigned char *data, const size_t len) {
    const unsigned char *first = data;
    const unsigned char *last  = first + len;

    size_t data_remaining;

    while(first < last) {
        if(!__packet) {
            __plen = _decodeLength(first, (last - first));
            __packet = new Packet;
            __packet->reserve(__plen);
        }

        data_remaining = __plen - __packet->size();
        data_remaining = (data_remaining > (last - first) ? (last - first) : data_remaining);

        __packet->writeAt(__packet->size(), first, data_remaining);
        first += data_remaining;

        if(__packet->size() == __plen) {
            onPacketReceived(__packet);
            __packet = nullptr;
        }
    }
}

void Socket::write(const Packet& p) {
    unsigned char d[8] = {0};
    size_t plen = p.size();
    size_t hlen = _encodeLength(plen, d);


    if( __type == TYPE::Client ) {
        d[hlen++] = __sequence++;
        if(__sequence > SEQUENCE_MAX)
            __sequence = SEQUENCE_MIN;
    }

    write(d, hlen, MSG_MORE);
    write(p.data(), plen);
}

void Socket::write(const unsigned char* data, const size_t len, int flags) const {
    if(__sock == -1)
        std::runtime_error("Socket is not connected");

    ::send(__sock, data, len, flags);
}

size_t Socket::_decodeLength(const unsigned char*& data, size_t len) {
    size_t c = 0;
    size_t r = 0;

    do
    {
        if(c > 4)
            throw std::out_of_range("Socket len error, too much bytes read.");

        if(c > len)
            throw std::out_of_range("Not enough data to decode len");

        r += (data[c] & 0x7f) << 7*c;
    } while(data[c++] & 0x80);

    data += c;

    // Skipping sequence from Client. Unchecked for now.
    if(__type == TYPE::Server)
        __sequence = *(data++);

    return r;
}

size_t Socket::_encodeLength(const size_t len, unsigned char* data) const {
    size_t c = 0;
    size_t l = len;

    do {
        if(c > 4)
            throw std::out_of_range("Socket encode length error");

        data[c] = (l & 0x7f);
        l >>= 7;

        if(l==0) break;

        data[c++] |= 0x80;
    } while( true );

    return c + 1;
}

void Socket::onPacketReceived(Packet*& p){
            __packets.push(p);
}

namespace Openmeat::Network {
    Socket& operator >>(Socket& s, Packet*& p) {
        if(s.__packets.size() == 0)
            throw std::out_of_range("No packet available");

        p = s.__packets.front();
        s.__packets.pop();
        return s;
    }

    Socket& operator <<(Socket& s, const Packet& p) {
        s.write(p);
        return s;
    }
}
