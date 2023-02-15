#include <exception>
#include "openmeat/packet"

using namespace Openmeat::Network;

// Force write operator
template PacketWriter& PacketWriter::operator<<<float>(float const&);
template PacketWriter& PacketWriter::operator<<<double>(double const&);
template PacketWriter& PacketWriter::operator<<<uint16_t>(uint16_t const&);
template PacketWriter& PacketWriter::operator<<<uint32_t>(uint32_t const&);
template PacketWriter& PacketWriter::operator<<<unsigned char>(unsigned char const&);

PacketWriter::PacketWriter() : __p(nullptr), __s(0) {}
PacketWriter::PacketWriter(Packet *p) : __p(p), __s(0) {}
PacketWriter::PacketWriter(Packet &p) : __p(&p), __s(0) {}

Packet::size_type PacketWriter::seek() const noexcept {
    return __s;
}

void PacketWriter::seek(const Packet::size_type s) noexcept {
    __s = s;
}

template<class T> PacketWriter& PacketWriter::operator<<(T const &e) {
    if(!__p)
        throw std::runtime_error("Pointer not set");

    __p->writeAt(__s, e);
    __s += sizeof(T);

    return *this;
}

template<> PacketWriter& PacketWriter::operator<<<std::string>(std::string const &e) {
    if(!__p)
        throw std::runtime_error("Pointer not set");

    __p->writeAt(__s, e);
    __s += e.length() + sizeof(uint16_t);

   return *this;
}
