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

#include "openmeat/packet"
#include "openmeat/socket"
#include "openmeat/opcodes"

using namespace Openmeat::Network;

const size_t KEYLEN = 20;

bool is_djb2_hash_equal(const int32_t &a, const int32_t &b) {
    return a == b || b^0xfc001fff == b;
}

int djb2_hash_comp(const int32_t &a, const int32_t &b) {
    if (is_djb2_hash_equal(a, b))
        return 0;

    return a < b ? 1 : -1;
}

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

    int break_msg_key(void) {
        int i;
        int32_t hash = INT_MAX;
        int32_t warray[KEYLEN] = {0};

        std::sort(key_bytes.begin(), key_bytes.end());
        auto it = std::unique(key_bytes.begin(), key_bytes.end());
        key_bytes.erase(it, key_bytes.end());

        std::cout << "Breaking with " << key_bytes.size() << " / " << KEYLEN << " known bytes..." << std::endl;
        do {
            _msgkey_bruteforce_worker(warray, hash++);
        } while (hash != INT_MAX);

        if (djb_hashes.empty()) {
            std::cerr << "No transitional DJB2 hash found" << std::endl;
            return 1;
        }

        std::sort(djb_hashes.begin(), djb_hashes.end(), djb2_hash_comp);
        auto uhash = std::unique(djb_hashes.begin(), djb_hashes.end(), is_djb2_hash_equal);
        djb_hashes.erase(uhash, djb_hashes.end());

        std::cout << "Found " << djb_hashes.size() << " possible transitional hash" << std::endl;
        for (const int32_t &h : djb_hashes) {
            generate_key(warray, h);
            std::cout << "int32_t msg_key[] = { ";
            for (i = 0; i < KEYLEN; i++)
                std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << warray[i] << ", ";
            std::cout << " };" << std::endl;
        }

        return 0;
    }

 protected:
    void onPacketReceived(Packet*& p) override {
        const unsigned char *data = p->data();

        if (p->opcode() == opcode_t::COMMUNITY)
            key_bytes.push_back(data[2]);

        delete p;
    }

    int32_t schwifty(int32_t h) {
        int32_t r = h;

        r ^= r << 13;
        r ^= r >> 17;
        r ^= r << 5;

        return r;
    }

    void generate_key(int32_t warray[KEYLEN], const int32_t hash) {
        int i;

        warray[0] = schwifty(hash);
        for (i=1; i < KEYLEN; i++)
            warray[i] = schwifty(warray[i-1]);
    }

    void _msgkey_bruteforce_worker(int32_t warray[KEYLEN], const int32_t hash) {
        int i;

        warray[0] = schwifty(hash);
        for (i=1; i < KEYLEN; i++) {
            if (!is_in_kbyte(warray[i-1] & 0xFF))
                return;

            warray[i] = schwifty(warray[i-1]);
        }

        if (is_in_kbyte(warray[KEYLEN-1] & 0xFF))
            djb_hashes.push_back(hash);
    }

    bool is_in_kbyte(unsigned char b) {
        for (const unsigned char &i : key_bytes)
            if ( b == i )
                return true;

        return false;
    }

 private:
    std::vector<int32_t> djb_hashes;
    std::vector<unsigned char> key_bytes;
};

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

    return parser.break_msg_key();
}
