#pragma once

#include <cstring>

// GCC10 doesn't ship std::bit_cast, provide this replacement until GCC11 is sufficiently available.

template <typename To, typename From>
constexpr To bit_cast(From from) {
    static_assert(sizeof(To) == sizeof(From));

    To to;
    std::memcpy(&to, &from, sizeof(From));
    return to;
}
