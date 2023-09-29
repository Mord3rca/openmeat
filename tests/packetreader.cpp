#include <cppunit/extensions/HelperMacros.h>

#include "openmeat/packet"

class NetworkPacketReaderTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NetworkPacketReaderTest);
    CPPUNIT_TEST(testSeek);
    CPPUNIT_TEST(testReadFloat);
    CPPUNIT_TEST(testReadDouble);
    CPPUNIT_TEST(testReadString);
    CPPUNIT_TEST(testReadUint16_t);
    CPPUNIT_TEST(testReadUint32_t);
    CPPUNIT_TEST(testReadUnsignedChar);
    CPPUNIT_TEST_EXCEPTION(testReadTooMuch, std::out_of_range);
    CPPUNIT_TEST_EXCEPTION(testSeekingTooMuch, std::out_of_range);
    CPPUNIT_TEST_SUITE_END();

public:
    void testSeek();
    void testReadFloat();
    void testReadDouble();
    void testReadString();
    void testReadUint16_t();
    void testReadUint32_t();

    void testReadUnsignedChar();
    void testReadTooMuch();
    void testSeekingTooMuch();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetworkPacketReaderTest);

using namespace Openmeat::Network;

std::initializer_list<unsigned char> test_read_data({
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
});

void NetworkPacketReaderTest::testReadFloat() {
    Packet s(test_read_data);
    PacketReader r(s);
    uint32_t t = 0x11223344, u = 0x55667788;
    float a, b;

    r >> a >> b;
    CPPUNIT_ASSERT_EQUAL(*reinterpret_cast<float*>(&t), a);
    CPPUNIT_ASSERT_EQUAL(*reinterpret_cast<float*>(&u), b);
}

void NetworkPacketReaderTest::testReadDouble() {
    Packet s(test_read_data);
    PacketReader r(s);
    uint64_t t = 0x1122334455667788;
    double a;

    r >> a;
    CPPUNIT_ASSERT_EQUAL(*reinterpret_cast<double*>(&t), a);
}

void NetworkPacketReaderTest::testReadString() {
    Packet s({0, 0x5, 'h', 'e', 'l', 'l', 'o'});
    PacketReader r(s);
    std::string a;

    r >> a;
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), a);
}

void NetworkPacketReaderTest::testReadUint16_t() {
    Packet s(test_read_data);
    PacketReader r(s);
    uint16_t a, b;

    r >> a >> b;
    CPPUNIT_ASSERT_EQUAL((uint16_t)0x1122, a);
    CPPUNIT_ASSERT_EQUAL((uint16_t)0x3344, b);
}

void NetworkPacketReaderTest::testReadUint32_t() {
    Packet s(test_read_data);
    PacketReader r(s);
    uint32_t a, b;

    r >> a >> b;
    CPPUNIT_ASSERT_EQUAL((uint32_t)0x11223344, a);
    CPPUNIT_ASSERT_EQUAL((uint32_t)0x55667788, b);
}


void NetworkPacketReaderTest::testReadUnsignedChar() {
    Packet s(test_read_data);
    PacketReader r(s);
    unsigned char a, b;

    r >> a >> b;
    CPPUNIT_ASSERT_EQUAL((unsigned char)0x11, a);
    CPPUNIT_ASSERT_EQUAL((unsigned char)0x22, b);
}

void NetworkPacketReaderTest::testSeek() {
    Packet s(test_read_data);
    PacketReader r(s);
    unsigned char a, b;

    r.seek(4);
    r >> a;

    r.seek(2);
    r >> b;

    CPPUNIT_ASSERT_EQUAL((unsigned char)0x55, a);
    CPPUNIT_ASSERT_EQUAL((unsigned char)0x33, b);
}

void NetworkPacketReaderTest::testReadTooMuch() {
    Packet s(test_read_data);
    PacketReader r(s);
    uint32_t a;

    r >> a >> a >> a;
}

void NetworkPacketReaderTest::testSeekingTooMuch() {
    Packet s(test_read_data);
    PacketReader r(s);
    uint32_t a;

    r.seek(12);
}
