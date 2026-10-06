#include <pcap/pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <fstream>
#include <iomanip>
#include <iostream>

#include "openmeat/packet"
#include "openmeat/socket"
#include "openmeat/opcodes"

using namespace Openmeat::Network;

const ssize_t KEYLEN = 20;
static unsigned char key[KEYLEN] = {0};

static union sequence {
    uint32_t s = 0;
    unsigned char b[4];
} seq;

class Parser : public Socket {
 public:
    explicit Parser(Socket::TYPE t) : Socket(t) {}

    int read_pcap_file(const char *filename) {
        pcap_t *pcap;
        size_t dataOffset;
        struct bpf_program fcode;
        const unsigned char *data;
        char errbuf[PCAP_ERRBUF_SIZE];
        struct pcap_pkthdr *pktheader;
        const struct tcphdr* tcpHeader;

        pcap = pcap_open_offline(filename, errbuf);
        if (!pcap) {
            std::cerr << "pcap_open_offline() failed: " << errbuf << std::endl;
            return 1;
        }

        pcap_compile(pcap, &fcode, "tcp", 1, 0xffffff);
        pcap_setfilter(pcap, &fcode);

        while (pcap_next_ex(pcap, &pktheader, &data) > 0) {
            // Filter force TCP packet so it's safe
            tcpHeader = (const struct tcphdr *)(data + sizeof(struct ether_header) + sizeof(struct ip));

            if ((ntohs(tcpHeader->dest) % 1000) != 801)
                continue;

            dataOffset = sizeof(struct ether_header) + sizeof(struct ip) + (tcpHeader->doff << 2);
            read(data + dataOffset, pktheader->len - dataOffset);
        }

        pcap_close(pcap);

        return 0;
    }

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
    int err;
    Parser parser(Socket::TYPE::Server);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    err = parser.read_pcap_file(argv[1]);
    if (err != 0)
        return err;

    std::cout << "Found " << seq.s << " community packets (more mean better accuracy)" << std::endl;
    // Yeah ... not precise enough. Looking for 0x00 in key should be better.
    if (seq.s > 20) {
        std::cout << "Printing key: " << std::endl;
        print_key();
    } else {
        std::cout << "Not enough community packets" << std::endl;
    }

    return 0;
}
