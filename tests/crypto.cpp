#include <cppunit/extensions/HelperMacros.h>

#include "openmeat/crypto"

using namespace Openmeat::Crypto;

class CryptoKeyTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CryptoKeyTests);
    CPPUNIT_TEST(testGenerate);
    CPPUNIT_TEST(testEquality);
    CPPUNIT_TEST(testInitProperty);
    CPPUNIT_TEST_SUITE_END();

 public:
    void testGenerate();
    void testEquality();
    void testInitProperty();
};
CPPUNIT_TEST_SUITE_REGISTRATION(CryptoKeyTests);

void CryptoKeyTests::testGenerate() {
    Key k = Key(0xbadc0ffe);

    CPPUNIT_ASSERT_EQUAL(0x5f599f8f, k[0]);
}

void CryptoKeyTests::testEquality() {
    Key k1 = Key(0xdeadbeef);
    Key k2 = Key(0xdeadbeef);

    CPPUNIT_ASSERT_EQUAL(k1, k2);
}

void CryptoKeyTests::testInitProperty() {
    Key k1 = Key(0xdeadbeef);
    Key k2 = Key(0xdeadbeef ^ 0xfc001fff);

    CPPUNIT_ASSERT_EQUAL(k1, k2);
}

constexpr Key tkey(0xdeadbeef);

const unsigned char unscrambled[] = "This is a test string...";
const unsigned char scrambled[] = {
    0x2c, 0xfb, 0xd9, 0x30, 0xb5, 0x5c, 0x49, 0x3d,
    0xb0, 0x86, 0x82, 0x2d, 0x44, 0x23, 0x84, 0xd8,
    0x1e, 0x47, 0x01, 0x80, 0xd8, 0x61, 0x5e, 0xf3
};

bool check_buffer_equality(const unsigned char *a, const unsigned char *b) {
    for (std::size_t i=0; i < 24; i++)
        if (*(a+i) != *(b+i))
            return false;

    return true;
}

class CryptoScrambleTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CryptoScrambleTests);
    CPPUNIT_TEST(testScramble);
    CPPUNIT_TEST(testUnscramble);
    CPPUNIT_TEST(testNoop);
    CPPUNIT_TEST_SUITE_END();

 public:
    void testScramble() {
        unsigned char buffer[24];
        memcpy(buffer, unscrambled, 24);
        scramble(tkey, reinterpret_cast<std::int32_t*>(buffer), 24 / sizeof(std::int32_t));
        CPPUNIT_ASSERT(check_buffer_equality(scrambled, buffer));
    }

    void testUnscramble() {
        unsigned char buffer[24];
        memcpy(buffer, scrambled, 24);
        unscramble(tkey, reinterpret_cast<std::int32_t*>(buffer), 24 / sizeof(std::int32_t));
        CPPUNIT_ASSERT(check_buffer_equality(unscrambled, buffer));
    }

    void testNoop() {
        unsigned char buffer[24];
        memcpy(buffer, unscrambled, 24);
        scramble(tkey, reinterpret_cast<std::int32_t*>(buffer), 24 / sizeof(std::int32_t));
        unscramble(tkey, reinterpret_cast<std::int32_t*>(buffer), 24 / sizeof(std::int32_t));

        CPPUNIT_ASSERT(check_buffer_equality(unscrambled, buffer));
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CryptoScrambleTests);
