#include <cppunit/extensions/HelperMacros.h>

#include "openmeat/packet"

class NetworkPacketWriterTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NetworkPacketWriterTest);
    CPPUNIT_TEST(testSeek);
    CPPUNIT_TEST(testWriteFloat);
    CPPUNIT_TEST(testWriteDouble);
    CPPUNIT_TEST(testWriteString);
    CPPUNIT_TEST(testWriteUint16_t);
    CPPUNIT_TEST(testWriteUint32_t);
    CPPUNIT_TEST(testWriteUnsignedChar);
    CPPUNIT_TEST_SUITE_END();

public:
    void testSeek();
    void testWriteFloat();
    void testWriteDouble();
    void testWriteString();
    void testWriteUint16_t();
    void testWriteUint32_t();
    void testWriteUnsignedChar();
};

CPPUNIT_TEST_SUITE_REGISTRATION( NetworkPacketWriterTest );

using namespace Openmeat::Network;

std::initializer_list<unsigned char> test_write_data({
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
});

void NetworkPacketWriterTest::testWriteFloat() {
    Packet e(test_write_data);
    Packet s; s.reserve(8);
    PacketWriter w(s);
    uint32_t t = 0x11223344;
    uint32_t u = 0x55667788;

    w << *reinterpret_cast<float*>(&t) << *reinterpret_cast<float*>(&u);
    CPPUNIT_ASSERT(e == s);
}

void NetworkPacketWriterTest::testWriteDouble() {
    Packet e(test_write_data);
    Packet s; s.reserve(8);
    PacketWriter w(s);
    uint64_t t = 0x1122334455667788;
    double a = *reinterpret_cast<double*>(&t);

    w << a;
    CPPUNIT_ASSERT(e == s);
}

void NetworkPacketWriterTest::testWriteString() {
    Packet e({0, 0x5, 'h', 'e', 'l', 'l', 'o'});
    Packet s; s.reserve(7);
    PacketWriter w(s);
    std::string a("hello");

    w << a;
    CPPUNIT_ASSERT(e == s);
}

void NetworkPacketWriterTest::testWriteUint16_t() {
    Packet e(test_write_data);
    Packet s; s.reserve(8);
    PacketWriter w(s);

    w << (uint16_t)0x1122 << (uint16_t)0x3344
      << (uint16_t)0x5566 << (uint16_t)0x7788;
    CPPUNIT_ASSERT(e == s);
}

void NetworkPacketWriterTest::testWriteUint32_t() {
    Packet e(test_write_data);
    Packet s; s.reserve(8);
    PacketWriter w(s);

    w << (uint32_t)0x11223344 << (uint32_t)0x55667788;
    CPPUNIT_ASSERT(e == s);
}


void NetworkPacketWriterTest::testWriteUnsignedChar() {
    Packet e(test_write_data);
    Packet s; s.reserve(8);
    PacketWriter w(s);

    w << (unsigned char)0x11 << (unsigned char)0x22
      << (unsigned char)0x33 << (unsigned char)0x44
      << (unsigned char)0x55 << (unsigned char)0x66
      << (unsigned char)0x77 << (unsigned char)0x88;
    CPPUNIT_ASSERT(e == s);
}

void NetworkPacketWriterTest::testSeek() {
    Packet e({0, 0, 0x33, 0, 0x55});
    Packet s; s.reserve(8);
    PacketWriter w(s);

    w.seek(4);
    w << (unsigned char)0x55;

    w.seek(2);
    w << (unsigned char)0x33;

    CPPUNIT_ASSERT(e == s);
}
