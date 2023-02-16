#include <cppunit/extensions/HelperMacros.h>

#include "openmeat/opcodes"

class NetworkUtilsTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NetworkUtilsTest);
    CPPUNIT_TEST(testOpcodeComparison);
    CPPUNIT_TEST_SUITE_END();

public:
    void testOpcodeComparison();
};

CPPUNIT_TEST_SUITE_REGISTRATION( NetworkUtilsTest );

using namespace Openmeat::Network;

void NetworkUtilsTest::testOpcodeComparison() {
    opcode_t disconnect = opcode_t::DISCONNECT;
    uint16_t op = 0x1a32;
    uint16_t op2 = 0x1a1a;

    CPPUNIT_ASSERT(op == disconnect);
    CPPUNIT_ASSERT(op2 != disconnect);
}
