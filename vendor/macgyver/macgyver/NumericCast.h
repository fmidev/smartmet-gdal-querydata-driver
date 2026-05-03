#pragma once

#include <limits>
#include "Exception.h"
#include "StringConversion.h"
#include "TypeTraits.h"

namespace Fmi {

/**
 * @brief Safely cast between numeric types with range checking
 * @tparam Target The target numeric type
 * @tparam Source The source numeric type
 * @param value The value to cast
 * @return The value cast to the target type
 * @throws Fmi::Exception if the value cannot be represented in the target type
 */
template <typename Target, typename Source>
typename std::enable_if<
    is_numeric<Target>::value && is_numeric<Source>::value &&
    !std::is_same<Target, Source>::value,
    Target>::type
numeric_cast(Source value)
{
    using type_t = std::common_type_t<Target, Source>;

    // Get min and max values for the target type
    constexpr type_t target_min = static_cast<type_t>(Fmi::lower_limit<Target>());
    constexpr type_t target_max = static_cast<type_t>(Fmi::upper_limit<Target>());

    bool own_error = false;

    // Helper function to format range error message
    const auto make_range_error = [&own_error](const auto& value, const auto& min, const auto& max, const char* type) {
        const std::string msg = "Value " + Fmi::to_string(value) +
               " out of range for " + type + " [" +
               Fmi::to_string(min) + "..." +
               Fmi::to_string(max) + "]";
        own_error = true;
        return msg;
    };

    try {
        // Special handling when Source is unsigned and Target is signed
        if constexpr (std::is_unsigned<Source>::value && std::is_signed<Target>::value) {
            if (value > target_max) {
                throw Fmi::Exception(BCP,
                    make_range_error(value, target_min, target_max, "signed target type"));
            }
        }
        // Special handling when Source is signed and Target is unsigned
        else if constexpr (std::is_signed<Source>::value && std::is_unsigned<Target>::value) {
            if (value < 0 || std::make_unsigned_t<type_t>(value) > target_max) {
                throw Fmi::Exception(BCP,
                    make_range_error(value, target_min, target_max, "unsigned target type"));
            }
        }
        // General case for other types
        else {
            if (value < target_min || value > target_max) {
                throw Fmi::Exception(BCP,
                    make_range_error(value, target_min, target_max, "target type"));
            }
        }

        return static_cast<Target>(value);
    }
    catch (const std::exception& e) {
        if (own_error) {
            // Simply rethrow our own (directly from this method but not from Fmi::to_string) error directly
            throw;
        }
        throw Fmi::Exception(BCP, "Numeric cast failed")
            .addDetail("Original error: " + std::string(e.what()));
    }
}

/**
 * @brief Specialization for same-type casts (no-op)
 */
template <typename T>
typename std::enable_if<is_numeric<T>::value, T>::type
numeric_cast(T value) noexcept
{
    return value;
}

} // namespace Fmi
