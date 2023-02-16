#include <cstdint>

#include "openmeat/opcodes"

bool Openmeat::Network::operator==(uint16_t const &a, opcode_t const &b) {
    return a == static_cast<uint16_t>(b);
}

bool Openmeat::Network::operator!=(uint16_t const &a, opcode_t const &b) {
    return a != static_cast<uint16_t>(b);
}
