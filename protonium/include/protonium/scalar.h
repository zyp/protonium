#pragma once

#include "scalar_encoders.h"


template <typename T, typename Encoder>
struct scalar_base {
    T n;

    constexpr scalar_base() : n() {}
    constexpr scalar_base(T n) : n(n) {}

    constexpr operator T() const {
        return n;
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End>
    bool decode(Iterator& it, End end) {
        return Encoder::decode(it, end, n);
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End>
    bool encode(Iterator& it, End end) const {
        return Encoder::encode(it, end, n);
    }

    constexpr bool is_present() const requires std::convertible_to<T, int> {
        return n != 0;
    }

    constexpr bool is_present() const requires (!std::convertible_to<T, int>) {
        return !n.empty();
    }

    operator std::span<uint8_t>() requires (!std::convertible_to<T, int>) {
        return n;
    }

    constexpr std::size_t size_bytes() const {
        return Encoder::size_bytes(n);
    }
};

using int32 = scalar_base<int32_t, varint>;
using int64 = scalar_base<int64_t, varint>;
using uint32 = scalar_base<uint32_t, varint>;
using uint64 = scalar_base<uint64_t, varint>;
using sint32 = scalar_base<int32_t, varint_zigzag>;
using sint64 = scalar_base<int64_t, varint_zigzag>;
using fixed32 = scalar_base<uint32_t, fixed>;
using fixed64 = scalar_base<uint64_t, fixed>;
using sfixed32 = scalar_base<int32_t, fixed>;
using sfixed64 = scalar_base<int64_t, fixed>;
using float_ = scalar_base<float, fixed>;
using double_ = scalar_base<double, fixed>;
using bool_ = scalar_base<bool, varint>;

template <typename Enum>
using enum_ = scalar_base<Enum, varint>;

template <typename StorageClass>
using bytes = scalar_base<typename StorageClass::container<uint8_t>, buffer>;

template <typename StorageClass>
using string = scalar_base<typename StorageClass::string, buffer>;

struct tag_t : uint32 {
    enum type_t : uint8_t {
        varint,
        fixed64,
        length_delimited,
        start_group,
        end_group,
        fixed32,
    };

    constexpr tag_t() {}
    constexpr tag_t(std::size_t num, type_t type) : uint32(static_cast<uint32_t>((num << 3) | type)) {}

    constexpr uint32_t num() {
        return n >> 3;
    }

    constexpr type_t type() {
        return static_cast<type_t>(n & 0x7);
    }
};

struct length_t : uint32 {
    constexpr length_t() {}
    constexpr length_t(std::size_t length) : uint32(static_cast<uint32_t>(length)) {}
};
