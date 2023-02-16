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

// Force Template generation
template void Packet::readAt<float>(Packet::size_type, float&) const;
template void Packet::readAt<double>(Packet::size_type, double&) const;
template void Packet::readAt<uint16_t>(Packet::size_type, uint16_t&) const;
template void Packet::readAt<uint32_t>(Packet::size_type, uint32_t&) const;
template void Packet::readAt<unsigned char>(Packet::size_type, unsigned char&) const;

template void Packet::writeAt<float>(Packet::size_type, float const&);
template void Packet::writeAt<double>(Packet::size_type, double const&);
template void Packet::writeAt<uint16_t>(Packet::size_type, uint16_t const&);
template void Packet::writeAt<uint32_t>(Packet::size_type, uint32_t const&);
template void Packet::writeAt<unsigned char>(Packet::size_type, unsigned char const&);

// Constant packets
const Packet Openmeat::Network::keepAlive({0x1a, 0x1a});


Packet::Packet() : std::vector<unsigned char>() {}

Packet::Packet(std::initializer_list<unsigned char> l) : std::vector<unsigned char>(l) {}

void Packet::reserve(Packet::size_type len) {
    if( len < MINIMAL_PACKET_SIZE )
        throw std::invalid_argument("Invalid packet size");

    std::vector<unsigned char>::reserve(len);
}


template<class T> void Packet::readAt(Packet::size_type pos, T &out) const {
    if( pos + sizeof(T) > size() )
        throw std::out_of_range("Packet: Not enough data to read");

    union {
        T e;
        unsigned char c[sizeof(T)];
    } u;

    #if __BYTE_ORDER == __LITTLE_ENDIAN
    for(size_t i = sizeof(T)-1;;i--)
    {
        u.c[i] = data()[pos++];
        if (i==0) break;
    }
    #elif __BYTE_ORDER == __BIG_ENDIAN
    #error "Not implemented yet"
    #else
    #error "Please set endianess"
    #endif

    out = u.e;
}

template<> void Packet::readAt<std::string>(Packet::size_type pos, std::string &str) const {
    uint16_t len;
    readAt<uint16_t>(pos, len);

    if(pos + sizeof(uint16_t) + len > size())
        throw std::out_of_range("Not enough data left to read string");

    str = std::string(reinterpret_cast<const char*>(data() + pos + sizeof(uint16_t)), len);
}

template<class T> void Packet::writeAt( Packet::size_type pos, T const &e) {
    if( pos + sizeof(T) > capacity() )
        throw std::out_of_range("Packet: Not enough space for write operation.");

    // Fill first bytes if not existing
    while(size() < pos) {
        push_back(0);
    }

    union {
        T e;
        unsigned char c[sizeof(T)];
    } u; u.e = e;

    #if __BYTE_ORDER == __LITTLE_ENDIAN
    for(size_t i = sizeof(T)-1;; i--) {
        writeAt(pos++, &u.c[i], 1);
        if(i == 0) break;
    }
    #elif __BYTE_ORDER == __BIG_ENDIAN
    #error "Not implemented yet"
    #else
    #error "Please set endianess"
    #endif
}

void Packet::writeAt(Packet::size_type pos, const unsigned char* s, const size_t len) {
    auto blen = pos + len;

    if( capacity() < blen )
        throw std::out_of_range("Packet: Not enough space for write operation.");

    while(size() < pos) {
        push_back(0);
    }

    for(auto i = 0; i < len; i++) {
        if(pos + i < size())
            data()[pos+i] = s[i];
        else
            push_back(s[i]);
    }
}

template<> void Packet::writeAt<std::string>(Packet::size_type pos, std::string const &e) {
    auto len = e.length();
    if( len > USHRT_MAX )
        throw std::out_of_range("Could not write string, size exceed 65535");

    writeAt<uint16_t>(pos, len);
    writeAt(pos + sizeof(uint16_t), reinterpret_cast<const unsigned char*>(e.c_str()), len);
}
