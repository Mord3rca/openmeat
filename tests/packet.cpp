#include <cppunit/extensions/HelperMacros.h>

#include "openmeat/packet"

class NetworkPacketTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NetworkPacketTest);
    CPPUNIT_TEST(testConstKeepAlive);
    CPPUNIT_TEST(testInitializerList);
    CPPUNIT_TEST_EXCEPTION(testOverflow, std::out_of_range);
    CPPUNIT_TEST_EXCEPTION(testStringOverflow, std::out_of_range);
    CPPUNIT_TEST(testResizeUp);
    CPPUNIT_TEST(testReadWriteFloat);
    CPPUNIT_TEST(testReadWriteDouble);
    CPPUNIT_TEST(testReadWriteUint16);
    CPPUNIT_TEST(testReadWriteUint32);
    CPPUNIT_TEST(testReadWriteUchar);
    CPPUNIT_TEST(testReadWriteString);
    CPPUNIT_TEST(testOpcode);
    CPPUNIT_TEST_SUITE_END();

 public:
    void testConstKeepAlive();
    void testInitializerList();
    void testOverflow();
    void testStringOverflow();
    void testResizeUp();
    void testResizeDown();
    void testReadWriteFloat();
    void testReadWriteDouble();
    void testReadWriteUint16();
    void testReadWriteUint32();
    void testReadWriteUchar();
    void testReadWriteString();
    void testOpcode();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetworkPacketTest);

using namespace Openmeat::Network;

void NetworkPacketTest::testConstKeepAlive() {
    Packet s;
    s.reserve(2);
    s.writeAt(0, (uint16_t)0x1a1a);

    CPPUNIT_ASSERT(s == keepAlive);
}

void NetworkPacketTest::testInitializerList() {
    std::initializer_list<unsigned char> const v({0xff, 0xee, 0xcc, 0xbb, 0xaa});
    Packet s(v); auto data = s.data();

    CPPUNIT_ASSERT(s.size() == v.size());
    for (auto const &i : v) {
        CPPUNIT_ASSERT(*data == i);
        data++;
    }
}

void NetworkPacketTest::testOverflow() {
    Packet s;
    uint32_t data = 0x00;

    s.reserve(8);
    s.writeAt(0, data); // This is fine
    s.writeAt(8, data); // But not this one
}

void NetworkPacketTest::testStringOverflow() {
    Packet s;
    std::string data("This is too much information.");

    s.reserve(8);
    s.writeAt(0, data);
}


void NetworkPacketTest::testResizeUp() {
    std::initializer_list<unsigned char> const expected(
        {0x77, 0x88, 0x11, 0x22, 0x33, 0x44});
    Packet s({0x77, 0x88});
    uint32_t data = 0x11223344;

    s.reserve(24);
    s.writeAt(2, data);

    s.reserve(32);

    auto d = s.data();
    CPPUNIT_ASSERT(s.size() == expected.size());
    for (auto const& i : expected) {
        CPPUNIT_ASSERT(i == *d);
        d++;
    }
}

void NetworkPacketTest::testReadWriteFloat() {
    float a = 2.4, b = 42.1337;
    float c, d;
    Packet s;

    s.reserve(sizeof(float)*2);
    s.writeAt(0, a);
    s.writeAt(sizeof(float), b);

    s.readAt(0, c);
    s.readAt(sizeof(float), d);

    CPPUNIT_ASSERT_EQUAL(a, c);
    CPPUNIT_ASSERT_EQUAL(b, d);
}

void NetworkPacketTest::testReadWriteDouble() {
    double a = 4.333333, b = 87.3;
    double c, d;
    Packet s;

    s.reserve(sizeof(double)*2);
    s.writeAt(0, a);
    s.writeAt(sizeof(double), b);

    s.readAt(0, c);
    s.readAt(sizeof(double), d);

    CPPUNIT_ASSERT_EQUAL(a, c);
    CPPUNIT_ASSERT_EQUAL(b, d);
}

void NetworkPacketTest::testReadWriteUint16() {
    uint16_t a = 0x1133, b = 0x2244;
    uint16_t c, d;
    Packet s;

    s.reserve(sizeof(uint16_t)*2);
    s.writeAt(0, a);
    s.writeAt(sizeof(uint16_t), b);

    s.readAt(0, c);
    s.readAt(sizeof(uint16_t), d);

    CPPUNIT_ASSERT_EQUAL(a, c);
    CPPUNIT_ASSERT_EQUAL(b, d);
}

void NetworkPacketTest::testReadWriteUint32() {
    uint32_t a = 0x33445566, b = 0x11227788;
    uint32_t c, d;
    Packet s;

    s.reserve(sizeof(uint32_t)*2);
    s.writeAt(0, a);
    s.writeAt(sizeof(uint32_t), b);

    s.readAt(0, c);
    s.readAt(sizeof(uint32_t), d);

    CPPUNIT_ASSERT_EQUAL(a, c);
    CPPUNIT_ASSERT_EQUAL(b, d);
}

void NetworkPacketTest::testReadWriteUchar() {
    unsigned char a = 0x24, b = 0x77;
    unsigned char c, d;
    Packet s;

    s.reserve(sizeof(unsigned char)*2);
    s.writeAt(0, a);
    s.writeAt(sizeof(unsigned char), b);

    s.readAt(0, c);
    s.readAt(sizeof(unsigned char), d);

    CPPUNIT_ASSERT_EQUAL(a, c);
    CPPUNIT_ASSERT_EQUAL(b, d);
}

void NetworkPacketTest::testReadWriteString() {
    std::string a("Hello"), b("World");
    std::string c, d;
    Packet s;

    s.reserve(sizeof(uint16_t)*2 + a.length() + b.length());
    s.writeAt(0, a);
    s.writeAt(a.length() + sizeof(uint16_t), b);

    s.readAt(0, c);
    s.readAt(c.length() + sizeof(uint16_t), d);

    CPPUNIT_ASSERT_EQUAL(a, c);
    CPPUNIT_ASSERT_EQUAL(b, d);
}

void NetworkPacketTest::testOpcode() {
    CPPUNIT_ASSERT_EQUAL(keepAlive.opcode(), opcode_t::KEEP_ALIVE);
}
