#include <exception>
#include "openmeat/packet"

using namespace Openmeat::Network;

// Force write operator
template PacketReader& PacketReader::operator>><float>(float&);
template PacketReader& PacketReader::operator>><double>(double&);
template PacketReader& PacketReader::operator>><uint16_t>(uint16_t&);
template PacketReader& PacketReader::operator>><uint32_t>(uint32_t&);
template PacketReader& PacketReader::operator>><unsigned char>(unsigned char&);

PacketReader::PacketReader() : __p(nullptr), __s(0) {}
PacketReader::PacketReader(Packet *p) : __p(p), __s(0) {}
PacketReader::PacketReader(Packet &p) : __p(&p), __s(0) {}

Packet::size_type PacketReader::seek() const noexcept {
    return __s;
}

void PacketReader::seek(const Packet::size_type s) {
   if (s > __p->size())
       throw std::out_of_range("Seek cannot exceed size()");

    __s = s;
}

template<class T> PacketReader& PacketReader::operator>>(T &e) {
    if (!__p)
        throw std::runtime_error("Pointer not set");

    __p->readAt(__s, e);
    __s += sizeof(T);

    return *this;
}

template<> PacketReader& PacketReader::operator>><std::string>(std::string &e) {
    if (!__p)
        throw std::runtime_error("Pointer not set");

    __p->readAt(__s, e);
    __s += e.length() + sizeof(uint16_t);

   return *this;
}
