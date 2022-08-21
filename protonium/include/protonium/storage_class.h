#pragma once

#include <vector>
#include <span>
#include <string>

namespace storage_class {
    struct dynamic {
        template <typename T>
        using container = std::vector<T>;

        using string = std::string;
    };

    template <std::size_t MaxSize>
    struct static_ {
        template <typename T>
        using container = std::array<T, MaxSize>; // TODO: replace std::array

        using string = std::array<char, MaxSize>; // TODO: replace std::array
    };

    struct ref {
        template <typename T>
        using container = std::span<T>;

        using string = std::string_view;
    };
}
