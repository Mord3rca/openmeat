#include "openmeat/packet"

#include <cstring>
#include <climits>
#include <iomanip>
#include <stdexcept>

extern "C"
{
    #include <bits/endian.h>
}

using namespace Openmeat::Network;

const Packet Openmeat::Network::keepAlive({0x1a, 0x1a});


Packet::Packet() : _data(nullptr), _datalen(0), _len(0), _seek(0)
{}

Packet::Packet(std::initializer_list<unsigned char> l) : Packet() {
    reserve(l.size());
    memcpy(_data, l.begin(), l.size());
    _len = l.size();
}

Packet::~Packet() {
    if( _data ) delete[] _data;
}

size_t Packet::length() const noexcept {
    return _len;
}

void Packet::reserve( size_t len ) {
    if( len < MINIMAL_PACKET_SIZE )
        throw std::invalid_argument("Invalid packet size");

    unsigned char* new_data = new unsigned char[ len ];
    size_t new_len = (len > _len) ? _len : 0 ;
    if( _data )
    {
        memcpy( new_data, _data, new_len );
        _seek = new_len;
        delete[] _data;
    }
    _len = new_len;
    _data = new_data;
    _datalen = len;
}

bool Packet::operator==(const Packet& p) const noexcept {
    if(p.length() != _len)
        return false;

    for(size_t i=0; i<_len; i++)
        if(p[i] != _data[i]) return false;

    return true;
}

const unsigned char* Packet::raw() const noexcept
{ return _data; }

const unsigned char Packet::operator []( size_t pos ) const {
    if( pos > _len )
        throw std::out_of_range("Packet: operator[] out of range.");

    return _data[pos];
}

void Packet::seek(const size_t s) noexcept {
    _seek = s;
}

const size_t Packet::seek() const noexcept {
    return _seek;
}

template<class T> T Packet::read() {
    if( length() < _seek + sizeof(T) )
        throw std::out_of_range("Packet: Not enough data to read");

    union {
        T e;
        unsigned char c[sizeof(T)];
    } u;

    #if __BYTE_ORDER == __LITTLE_ENDIAN
    for(size_t i = sizeof(T)-1;;i--)
    {
        u.c[i] = _data[_seek++];
        if (i==0) break;
    }
    #elif __BYTE_ORDER == __BIG_ENDIAN
    #error "Not implemented yet"
    #else
    #error "Please set endianess"
    #endif

    return u.e;
}

namespace Openmeat::Network {
    Packet& operator >>(Packet& p, float& s) {
        s = p.read<float>();
        return p;
    }

    Packet& operator >>(Packet& p, double& s) {
        s = p.read<double>();
        return p;
    }

    Packet& operator >>(Packet& p, uint16_t& s) {
        s = p.read<uint16_t>();
        return p;
    }

    Packet& operator >>(Packet& p, uint32_t& s) {
        s = p.read<uint32_t>();
        return p;
    }

    Packet& operator >>(Packet& p, std::string& s) {
        auto len = p.read<uint16_t>();
        if ( p._seek + len > p._datalen)
            throw std::out_of_range("Not enough data left to read string");

        s = std::string(reinterpret_cast<const char*>(p._data + p._seek), len);
        p._seek += len;
        return p;
    }

    Packet& operator >>(Packet& p, unsigned char& s) {
        s = p.read<unsigned char>();
        return p;
    }
}

void Packet::write(const unsigned char* s, const size_t len) {
    if( _datalen < _seek + len )
        throw std::out_of_range("Packet: Not enough space for write operation.");

    memcpy(_data + _seek, s, len);
    _seek += len;

    if(_seek > _len) _len = _seek;
}

template<class T> void Packet::write( T const &e ) {
    if( _datalen < _seek + sizeof(T) )
        throw std::out_of_range("Packet: Not enough space for write operation.");

    union {
        T e;
        unsigned char c[sizeof(T)];
    } u; u.e = e;

    #if __BYTE_ORDER == __LITTLE_ENDIAN
    for(size_t i = sizeof(T)-1;; i--) {
        _data[_seek++] = u.c[i];
        if(i == 0) break;
    }
    #elif __BYTE_ORDER == __BIG_ENDIAN
    #error "Not implemented yet"
    #else
    #error "Please set endianess"
    #endif

    if(_seek > _len) _len = _seek;
}

template<> void Packet::write<std::string>(const std::string &e) {
    auto len = e.length();

    if( len > USHRT_MAX )
        throw std::out_of_range("Could not write string, size exceed 65535");

    write<uint16_t>(len);
    write(reinterpret_cast<const unsigned char*>(e.c_str()), len);
}


namespace Openmeat::Network {
    Packet& operator<<(Packet& p, float const& e) {
        p.write<float>(e);
        return p;
    }

    Packet& operator<<(Packet& p, double const& e) {
        p.write<double>(e);
        return p;
    }

    Packet& operator<<(Packet& p, uint16_t const& e) {
        p.write<uint16_t>(e);
        return p;
    }

    Packet& operator<<(Packet& p, uint32_t const& e) {
        p.write<uint32_t>(e);
        return p;
    }

    Packet& operator<<(Packet& p, std::string const& e) {
        p.write<std::string>(e);
        return p;
    }

    Packet& operator<<(Packet& p, unsigned char const& e) {
        p.write<unsigned char>(e);
        return p;
    }
}
