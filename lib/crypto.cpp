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
    stream << "[";
    for (Key::size_type i = 0; i < k.size(); i++) {
        stream << " "  << std::setw(8) << std::setfill('0') << std::hex << k[i];
        if (i != (k.size()-1))
            stream << ",";
    }
    stream << " ]";

    return stream;
}

bool Openmeat::Crypto::scramble(const Key &key, std::int32_t data[], const std::size_t len) {
    std::int32_t i, j;
    std::int32_t a, b, c, d, e;

    if (len < 2)
        return false;

    a = data[len - 1];
    b = data[0];
    c = d = 6 + (52 / len);
    e = 0;

    i = c;
    while (i-- > 0) {
        e += 2654435769;
        d = ((unsigned) e >> 2) & 3;

    for (j = 0; j < len; j++) {
            b = data[(j + 1) % len];
            c = \
                ((((unsigned) a >> 5) ^ (b << 2)) + (((unsigned) b >> 3) ^ (a << 4))) ^ \
                ((e ^ b) + (key[(j & 3) ^ d] ^ a));
            data[j] += c;
            a = data[j];
        }
    }

    return true;
}

bool Openmeat::Crypto::unscramble(const Key &key, std::int32_t data[], const std::size_t len) {
    std::int32_t i, j;
    std::int32_t a, b, c, d, e;

    if (len < 2)
        return false;

    a = data[len-2];
    b = data[0];
    c = d = 6 + (52 / len);
    e = 2654435769 * c;

    i = c;
    while (i-- > 0) {
        d = ((unsigned) e >> 2) & 3;

        for (j=len - 1; j >= 0; j--) {
            a = data[(j - 1 < 0) ? len-1 : j-1];
            c = \
                ((((unsigned) a >> 5) ^ (b << 2)) + (((unsigned) b >> 3) ^ (a << 4))) ^ \
                ((e ^ b) + (key[(j & 3) ^ d] ^ a));
            data[j] -= c;
            b = data[j];
        }

        e -= 2654435769;
    }

    return true;
}
