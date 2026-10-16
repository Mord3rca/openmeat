#include <iomanip>

#include "openmeat/crypto"

using namespace Openmeat::Crypto;

bool Openmeat::Crypto::operator==(Key const& lhs, Key const& rhs) {
    for (Key::size_type i = 0; i < lhs.size(); i++)
        if (lhs[i] != rhs[i])
            return false;

    return true;
}

std::ostream& Openmeat::Crypto::operator<<(std::ostream& stream, Key const& k) {
    stream << "[ ";
    for (Key::size_type i = 0; i < k.size(); i++) {
        stream << std::setw(8) << std::setfill('0') << std::hex << k[i];
        if (i == (k.size()-1))
            stream << " ";
        else
            stream << ", ";
    }
    stream << "]";

    return stream;
}
