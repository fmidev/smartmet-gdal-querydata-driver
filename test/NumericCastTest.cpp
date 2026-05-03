#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "NumericCast.h"  // Assuming this is the header with our implementation

TEST_CASE("numeric_cast basic functionality", "[numeric_cast]") {
    SECTION("Same type casts") {
        REQUIRE(Fmi::numeric_cast<int>(42) == 42);
        REQUIRE(Fmi::numeric_cast<double>(3.14) == 3.14);
    }

    SECTION("Valid integer conversions") {
        REQUIRE(Fmi::numeric_cast<int32_t>(int16_t{100}) == 100);
        REQUIRE(Fmi::numeric_cast<int64_t>(int32_t{1000000}) == 1000000);
        REQUIRE(Fmi::numeric_cast<int16_t>(int8_t{50}) == 50);
    }

    SECTION("Valid floating point to integer conversions") {
        REQUIRE(Fmi::numeric_cast<int32_t>(2.0f) == 2);
        REQUIRE(Fmi::numeric_cast<int64_t>(3.0) == 3);
    }
}

TEST_CASE("numeric_cast signed/unsigned conversions", "[numeric_cast]") {
    SECTION("Unsigned to signed valid conversions") {
        REQUIRE(Fmi::numeric_cast<int32_t>(uint16_t{1000}) == 1000);
        REQUIRE(Fmi::numeric_cast<int64_t>(uint32_t{1000000}) == 1000000);
    }

    SECTION("Signed to unsigned valid conversions") {
        REQUIRE(Fmi::numeric_cast<uint32_t>(int16_t{1000}) == 1000);
        REQUIRE(Fmi::numeric_cast<uint64_t>(int32_t{1000000}) == 1000000);
    }
}

TEST_CASE("numeric_cast overflow checks", "[numeric_cast]") {
    SECTION("Unsigned to signed overflow") {
        uint32_t large_unsigned = std::numeric_limits<int32_t>::max();
        large_unsigned++;  // Now larger than max signed int32

        REQUIRE_THROWS_AS(
            Fmi::numeric_cast<int32_t>(large_unsigned),
            Fmi::Exception
        );
    }

    SECTION("Signed to unsigned negative value") {
        REQUIRE_THROWS_AS(
            Fmi::numeric_cast<uint32_t>(int32_t{-1}),
            Fmi::Exception
        );
    }

    SECTION("Integer overflow") {
        int32_t large_int = std::numeric_limits<int16_t>::max();
        large_int++;  // Now larger than max int16

        REQUIRE_THROWS_AS(
            Fmi::numeric_cast<int16_t>(large_int),
            Fmi::Exception
        );

        REQUIRE_THROWS_AS(
            Fmi::numeric_cast<int64_t>(std::numeric_limits<uint64_t>::max()),
            Fmi::Exception
        );
    }
}

TEST_CASE("numeric_cast floating point checks", "[numeric_cast]") {
    SECTION("Float to integer overflow") {
        float large_float = static_cast<float>(std::numeric_limits<int32_t>::max()) * 2.0f;

        REQUIRE_THROWS_AS(
            Fmi::numeric_cast<int32_t>(large_float),
            Fmi::Exception
        );
    }

    SECTION("Double to float potential precision loss") {
        double precise_value = 1.23456789012345;
        float float_value = Fmi::numeric_cast<float>(precise_value);

        // Verify the cast succeeds but expect some precision loss
        REQUIRE(std::abs(float_value - precise_value) < 1e-6f);
    }
}

TEST_CASE("numeric_cast edge cases", "[numeric_cast]") {
    SECTION("Maximum values") {
        REQUIRE(Fmi::numeric_cast<int32_t>(std::numeric_limits<int16_t>::max()) 
                == std::numeric_limits<int16_t>::max());
    }

    SECTION("Minimum values") {
        REQUIRE(Fmi::numeric_cast<int32_t>(std::numeric_limits<int16_t>::min()) 
                == std::numeric_limits<int16_t>::min());
    }

    SECTION("Zero conversions") {
        REQUIRE(Fmi::numeric_cast<uint32_t>(0) == 0);
        REQUIRE(Fmi::numeric_cast<int32_t>(0u) == 0);
        REQUIRE(Fmi::numeric_cast<float>(0) == 0.0f);
    }
}
