#include <fstream>
#include <iomanip>
#include <iostream>

#include "openmeat/packet"
#include "openmeat/socket"
#include "openmeat/opcodes"

using namespace Openmeat::Network;

const ssize_t KEYLEN = 20;
// Client conversation is expected, packets are small
const ssize_t BUFFER_SIZE = 512;

static unsigned char key[KEYLEN] = {0};
static unsigned char buffer[BUFFER_SIZE];

static union sequence {
    uint32_t s = 0;
    unsigned char b[4];
} seq;

class Parser : public Socket {
 public:
    Parser(Socket::TYPE t) : Socket(t) {}

 protected:
    void onPacketReceived(Packet*& p) override {
        // number is right after the comunity command
        // which is a uint16_t so we offset the position
        auto pos = sequence() + 2;
        const unsigned char *data = p->data();

        if (p->opcode() != opcode_t::COMMUNITY)
            goto end;
        seq.s++;

        for (auto i = 0; i < 4; i++) {
            key[(pos+i) % KEYLEN] = data[4+i] ^ seq.b[3-i];
        }

    end:
        delete p;
    }
};

void print_key() {
    std::cout << "unsigned char key[" << KEYLEN << "] = { "
        << std::setfill('0') << std::setw(2) << std::hex;
    for (auto i = 0; i < KEYLEN; i++)
        std::cout << "0x" << (ushort)key[i] << ", ";
    std::cout << "};" << std::endl;
}

int main(int argc, char *argv[]) {
    std::ifstream file;
    Parser parser(Socket::TYPE::Server);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    file.open(argv[1], std::ifstream::in | std::ifstream::binary);
    while (file.good()) {
        auto len = file.readsome(reinterpret_cast<char*>(buffer), BUFFER_SIZE);

        if (len == 0)
            break;

        parser.read(buffer, len);
    }
    file.close();

    std::cout << "Found " << seq.s << " community packets (more mean better accuracy)" << std::endl;
    // Yeah ... not precise enough. Looking for 0x00 in key should be better.
    if (seq.s > 20) {
        std::cout << "Printing key: " << std::endl;
        print_key();
    } else
        std::cout << "Not enough community packets" << std::endl;

    return 0;
}
