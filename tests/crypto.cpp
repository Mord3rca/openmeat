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

