#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <algorithm>
#include <cstdint>
#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "openmeat/crypto"
#include "openmeat/packet"
#include "openmeat/socket"
#include "openmeat/opcodes"

using namespace Openmeat::Crypto;
using namespace Openmeat::Network;

const size_t KEYLEN = 20;

bool is_djb2_hash_equal(const int32_t a, const int32_t b) {
    return a == b || a == b^0xfc001fff;
}

int djb2_hash_comp(const int32_t a, const int32_t b) {
    if (is_djb2_hash_equal(a, b))
        return 0;

    return a - b;
}

void reduce_djb2_hashes(std::vector<int32_t> &h) {
    std::sort(h.begin(), h.end(), djb2_hash_comp);
    auto uhash = std::unique(h.begin(), h.end(), is_djb2_hash_equal);
    h.erase(uhash, h.end());
}

class PartialKey : public Key {
 public:
    PartialKey() : Key(0) {}

    void reset(const std::int32_t hash) {
        (*this)[0] = _nhash(hash);
    }

    bool equal(const unsigned char data[KEYLEN]) {
        std::size_t i;

        if (data[0] && ((*this)[0] & 0xff) != data[0])
            return false;

        for (i=1; i < KEYLEN; i++) {
            (*this)[i] = _nhash((*this)[i-1]);
            if (data[i] && ((*this)[i] & 0xff) != data[i])
                return false;
        }

        return true;
    }
};

class Breaker : public Socket {
 public:
    explicit Breaker(Socket::TYPE t) : Socket(t), community_count(1), kbytes{0x00} {}

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

        (void)pcap_compile(pcap, &fcode, "tcp", 1, 0xffffff);
        (void)pcap_setfilter(pcap, &fcode);

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

    int break_msg_key(void) {
        PartialKey k;
        std::int64_t hash;

        if (std::count(kbytes.begin(), kbytes.end(), 0) > 12 /* Gate number ... can be modified. */) {
            std::cerr << "Not enough leaked bytes to crack MSG Key, aborting..." << std::endl;
            return 1;
        }

        for (hash = 0; hash < 0x100000000; hash++) {
            k.reset(hash);
            if (k.equal(kbytes.data()))
                djb_hashes.push_back(hash);
        }

        if (djb_hashes.empty()) {
            std::cerr << "No transitional DJB2 hash found for msg key, aborting" << std::endl;
            return 1;
        }

        reduce_djb2_hashes(djb_hashes);

        std::cout << "MSG Key Found !" << std::endl
                  << "Use the following definition(s) to import it in your project:" << std::endl << std::endl;
        for (const auto &i : djb_hashes)
            std::cout << "Openmeat::Crypto::Key mkey(" << i << ");" << std::endl;

        return 0;
    }

 protected:
    void onPacketReceived(Packet*& p) override {
        unsigned char pos;
        unsigned char *data = p->data(), *cur;

        if (p->opcode() != opcode_t::COMMUNITY)
            goto end;

        if (p->size() < 8) /* opcode + community_subcmd + sequence */
            throw std::out_of_range("Community packet don't have enough data");

        pos = sequence() + 3;  // (seq() + 1) + community subcmd, no opcode (since not encrypted)

        *reinterpret_cast<std::uint32_t*>(data + 4) ^= htonl(community_count);
        for (unsigned char i = 0; i < sizeof(std::uint32_t); i++) {
            cur = data + 4 + i;

            if (kbytes[(pos+i) % KEYLEN] != 0 && *cur != kbytes[(pos+i) % KEYLEN])
                throw std::runtime_error("A previous set mkey byte did not match new found value, aborting");

            kbytes[(pos+i) % KEYLEN] = *cur;
        }
        community_count++;

    end:
        delete p;
    }

 private:
    std::uint32_t community_count;
    std::array<unsigned char, KEYLEN> kbytes;

    std::vector<int32_t> djb_hashes;
};

int main(int argc, char *argv[]) {
    int err;
    Breaker breaker(Socket::TYPE::Server);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    err = breaker.read_pcap_file(argv[1]);
    if (err != 0)
        return err;

    return breaker.break_msg_key();
}
