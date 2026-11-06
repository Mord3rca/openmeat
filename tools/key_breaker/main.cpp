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
    return a == b || a == (b^0xfc001fff);
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

void apply_ntohl(std::int32_t arr[], std::size_t len) {
    for (std::size_t i = 0; i < len; i ++)
        arr[i] = ntohl(arr[i]);
}

class PartialKey : public Key {
 public:
    PartialKey() : Key(0) {}

    void reset(const std::int32_t hash) noexcept {
        (*this)[0] = _nhash(hash);
    }

    void init(const std::int32_t hash, const std::size_t len) noexcept {
        (*this)[0] = _nhash(hash);

        for (int i = 1; i < len; i++)
            (*this)[i] = _nhash((*this)[i-1]);
    }

    void init_unscramble(const std::int32_t hash) noexcept {
        init(hash, 4);
    }

    bool equal(const unsigned char data[KEYLEN]) noexcept {
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

    std::int32_t djb2_hash(std::int32_t hash, const char r[]) noexcept {
        init(hash, size());
        return _djb2_hash(r);
    }
};

class Breaker : public Socket {
 public:
    explicit Breaker(Socket::TYPE t) : Socket(t), community_count(1), kbytes{0x00}, ident(nullptr) {}

    ~Breaker() {
        if (ident)
            delete ident;
    }

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
                msg_djb_hashes.push_back(hash);
        }

        reduce_djb2_hashes(msg_djb_hashes);

        return 0;
    }

    int break_ident_key(void) {
        Packet wpack;
        PartialKey k;
        std::int64_t hash;

        for (hash = 0; hash < 0x100000000; hash++) {
            k.init_unscramble(hash);
            if (_check_ident_packet(wpack, k))
                ident_djb_hashes.push_back(hash);
        }

        reduce_djb2_hashes(ident_djb_hashes);

        return 0;
    }

    static bool is_in_vector(const std::vector<std::int32_t> &v, const std::int32_t hash) {
        for (std::size_t i = 0; i < v.size(); i++) {
            if (is_djb2_hash_equal(v[i], hash))
                return true;
        }
        return false;
    }

    bool try_reduce_complexity() {
        PartialKey k;
        std::int32_t djb;
        std::int64_t hash;

        for (hash=0; hash < 0x100000000; hash++) {
            djb = k.djb2_hash(hash, "msg");
            if (is_in_vector(msg_djb_hashes, djb))
                root_djb_hashes.push_back(hash);
        }

        reduce_djb2_hashes(root_djb_hashes);

        return !root_djb_hashes.empty();
    }

    bool break_ident_key_lcomplexity() {
        PartialKey k;
        Packet wpack;
        std::int32_t djb;
        std::vector<std::int32_t> valid_root_hashes;

        if (!ident)
            return -1;

        for (const std::int32_t &h : root_djb_hashes) {
            djb = k.djb2_hash(h, "identification");
            k.init(djb, k.size());
            if (!_check_ident_packet(wpack, k))
                continue;

            ident_djb_hashes.push_back(djb);
            valid_root_hashes.push_back(h);
        }

        root_djb_hashes = valid_root_hashes;
        reduce_djb2_hashes(ident_djb_hashes);

        return !ident_djb_hashes.empty();
    }

    int break_all() {
        int err = 0;

        std::cout << "Breaking MSG Key ..." << std::endl;
        err = break_msg_key();
        if (!ident) {
            std::cout << "No LOGIN packet found in capture, stopping there." << std::endl;
            print_result();
            return err;
        }

        std::cout << "Trying to reduce crack complexity based on MSG Key crack result ..." << std::endl;
        if (try_reduce_complexity()) {
            std::cout << "Breaking IDENTIFICATION Key with low complexity algorithm ..." << std::endl;
            if (break_ident_key_lcomplexity()) {
                print_result();
                return 0;
            }
            std::cout << "Failed to find a root key." << std::endl;
        }

        std::cout << "Breaking IDENTIFICATION Key ..." << std::endl;
        err = break_ident_key();

        print_result();
        return err;
    }

    void print_result() {
        std::cout << "=== RESULT ===" << std::endl << std::endl;

        if (!root_djb_hashes.empty()) {
            std::cout << "constexpr Openmeat::Crypto::Key rkey(" << root_djb_hashes[0] << ");" << std::endl;
            std::cout << "constexpr Openmeat::Crypto::Key mkey = rkey.derive(\"msg\");"
                      << "  // Calculated hash: " << msg_djb_hashes[0] << std::endl
                      << "constexpr Openmeat::Crypto::Key ikey = rkey.derive(\"identification\");"
                      << "  // Calculated hash: " << ident_djb_hashes[0] << std::endl;
            return;
        }

        if (!msg_djb_hashes.empty()) {
            std::cout << "constexpr Openmeat::Crypto::Key mkey(" << msg_djb_hashes[0] << ");" << std::endl;
        }

        if (!ident_djb_hashes.empty()) {
            std::cout << "constexpr Openmeat::Crypto::Key ikey(" << ident_djb_hashes[0] << ");" << std::endl;
        }

        return;
    }

 protected:
    void onPacketReceived(Packet*& p) override {
        unsigned char pos;
        unsigned char *data = p->data(), *cur;

        switch (p->opcode()) {
        case opcode_t::LOGIN:
            if (ident)
                break;

            p->readAt<uint16_t>(2, ident_len);
            if (ident_len << 2 != p->size() - 4 /* -opcode -arrayLen */) {
                std::cerr << "WARN: Malformed LOGIN packet with sequence " << sequence() << std::endl;
                break;
            }
            apply_ntohl(reinterpret_cast<std::int32_t*>(p->data() + 4), ident_len);
            ident = p;
            return;

        case opcode_t::COMMUNITY:
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

        default:
            ;;
        }

        delete p;
    }

    bool _check_ident_packet(Packet &w, Key &k) {
        w = *ident;

        unscramble(k, reinterpret_cast<std::int32_t*>(w.data() + 4), ident_len);
        apply_ntohl(reinterpret_cast<std::int32_t*>(w.data() + 4), ident_len);
        return _check_ident_validity(w);
    }

    bool _check_ident_validity(const Packet &p) {
        PacketReader r(p);
        std::uint16_t size;

        try {
            r.seek() = 4;
            r >> size; r.seek() += size;
            r >> size; r.seek() += size;
            r >> size; r.seek() += size;
            r >> size; r.seek() += size;
            r.seek() += 7; /* int + short + byte */
            r >> size; r.seek() += size;
        } catch (std::out_of_range &e) {
            return false;
        }

        return (p.size() - r.seek()) < 4;
    }

 private:
    std::vector<int32_t> root_djb_hashes;

    std::uint32_t community_count;
    std::array<unsigned char, KEYLEN> kbytes;
    std::vector<int32_t> msg_djb_hashes;

    Packet *ident;
    std::uint16_t ident_len;
    std::vector<int32_t> ident_djb_hashes;
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

    return breaker.break_all();
}
