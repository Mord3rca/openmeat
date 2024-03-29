#include <cppunit/extensions/HelperMacros.h>

#include "openmeat/socket"

class NetworkSocketTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NetworkSocketTest);
    CPPUNIT_TEST(testReadClientPacket);
    CPPUNIT_TEST(testReadServerPacket);
    CPPUNIT_TEST(testLongRead);
    CPPUNIT_TEST(testFragmentedRead);
    CPPUNIT_TEST(testMultipleRead);
    CPPUNIT_TEST_SUITE_END();

 public:
    void testReadClientPacket();
    void testReadServerPacket();
    void testLongRead();
    void testFragmentedRead();
    void testMultipleRead();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetworkSocketTest);

using namespace Openmeat::Network;

void NetworkSocketTest::testReadClientPacket() {
    Socket s(Socket::TYPE::Client); Packet *p;
    const unsigned char data[] = {0x02, 0x1a, 0x1a};

    s.read(data, sizeof(data));
    s >> p;

    CPPUNIT_ASSERT(*p == keepAlive);

    delete p;
}

void NetworkSocketTest::testReadServerPacket() {
    Socket s(Socket::TYPE::Server); Packet *p;
    const unsigned char data[] = {0x04, 0x55, 0x22, 0x33, 0x44, 0x55};
    uint32_t i;

    s.read(data, sizeof(data));
    s >> p;

    p->readAt(0, i);

    CPPUNIT_ASSERT(p->size() == 4);
    CPPUNIT_ASSERT(i == 0x22334455);

    delete p;
}

const unsigned char LONGREAD_DATA[] = {
0xdd, 0x02, 0xFF, 0xAA, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

void NetworkSocketTest::testLongRead() {
    Socket s(Socket::TYPE::Server); Packet *p;
    uint16_t opcode;

    s.read(LONGREAD_DATA, sizeof(LONGREAD_DATA));
    s >> p;

    p->readAt(0, opcode);

    CPPUNIT_ASSERT(p->size() == 349);
    CPPUNIT_ASSERT(opcode == 0xaabb);

    delete p;
}

void NetworkSocketTest::testFragmentedRead() {
    Socket s; Packet *p;
    const unsigned char data1[] = {0x02};
    const unsigned char data2[] = {0x1a, 0x1a};

    s.read(data1, sizeof(data1));
    s.read(data2, sizeof(data2));

    s >> p;

    CPPUNIT_ASSERT(keepAlive == *p);
    delete p;
}

void NetworkSocketTest::testMultipleRead() {
    Socket s; Packet *p;
    const unsigned char data[] = {
        0x02, 0x1a, 0x1a,
        0x02, 0x1a, 0x1a,
        0x02, 0x1a, 0x1a,
        0x02, 0x1a, 0x1a,
    };
    s.read(data, sizeof(data));

    for (int i = 0; i < 4; i++) {
        s >> p;
        CPPUNIT_ASSERT(*p == keepAlive);
        delete p;
    }
}
