#include <boost/test/unit_test.hpp>

#include "../include/base64.h"

BOOST_AUTO_TEST_SUITE(base64_test_suite)

    BOOST_AUTO_TEST_CASE(EncodeTest)
    {
        const std::string s = "test";
        BOOST_CHECK_EQUAL(base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length()), "dGVzdA==");
    }

    BOOST_AUTO_TEST_CASE(DecodeTest)
    {
        const std::string s = "dGVzdA==";
        BOOST_CHECK_EQUAL(base64_decode(s), "test");
    }

    BOOST_AUTO_TEST_CASE(EncodeDecodeTest)
    {
        const std::string s = "test";
        
        std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
        std::string decoded = base64_decode(encoded);

        BOOST_CHECK_EQUAL(decoded, s);
    }

BOOST_AUTO_TEST_SUITE_END()