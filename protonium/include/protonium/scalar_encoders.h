#pragma once

#include <iterator>
#include <cstdint>

struct varint {
    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, std::unsigned_integral T>
    static bool encode(Iterator& it, End end, T n) {
        while(it != end) {
            uint8_t b = n & 0x7f;
            
            if(n <= 0x7f) {
                *it++ = b;
                return true;
            }

            *it++ = b | 0x80;
            n >>= 7;
        }

        return false; // Error: output overflow.
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, std::signed_integral T>
    static bool encode(Iterator& it, End end, T n) {
        return encode(it, end, static_cast<uint64_t>(static_cast<int64_t>(n)));
    }

    template <typename T>
    static bool encode(auto& it, auto end, T n) requires std::is_enum_v<T> {
        return encode(it, end, static_cast<int32_t>(n));
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End, std::unsigned_integral T>
    static bool decode(Iterator& it, End end, T& n) {
        std::size_t shift = 0;
        n = 0;
        
        while(it != end && shift < sizeof(T) * 8) {
            uint8_t b = *it++;

            n |= static_cast<T>(b & 0x7f) << shift;

            if((b & 0x80) == 0) {
                return true;
            }

            shift += 7;
        }

        return false; // Error: input underflow or integer overflow.
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End, std::signed_integral T>
    static bool decode(Iterator& it, End end, T& n) {
        uint64_t u;
        auto ret = decode(it, end, u);
        n = static_cast<T>(u);
        return ret;
    }

    template <typename T>
    static bool decode(auto& it, auto end, T& n) requires std::is_enum_v<T> {
        uint64_t u;
        auto ret = decode(it, end, u);
        n = static_cast<T>(u);
        return ret;
    }

    template <std::integral T>
    constexpr static std::size_t size_bytes(T n) {
        if constexpr(std::is_signed_v<T>) {
            if(n < 0) {
                return 10;
            }
        }

        for(std::size_t i = 1; i < 10; i++) {
            if(static_cast<uint64_t>(n) < (1ull << (7 * i))) {
                return i;
            }
        }

        return 10;
    }

    template <typename T>
    constexpr static std::size_t size_bytes(T n) requires std::is_enum_v<T> {
        return size_bytes(static_cast<int32_t>(n));
    }
};

struct varint_zigzag {
    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, std::signed_integral T>
    static bool encode(Iterator& it, End end, T n) {
        return varint::encode(it, end, static_cast<std::make_unsigned_t<T>>(n < 0 ? ((-n) << 1) - 1 : n << 1));
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End, std::signed_integral T>
    static bool decode(Iterator& it, End end, T& n) {
        std::make_unsigned_t<T> u;
        auto ret = varint::decode(it, end, u);
        n = u & 1 ? -static_cast<T>(u >> 1) - 1 : static_cast<T>(u >> 1);
        return ret;
    }

    template <std::signed_integral T>
    constexpr static std::size_t size_bytes(T n) {
        return varint::size_bytes(static_cast<std::make_unsigned_t<T>>(n < 0 ? ((-n) << 1) - 1 : n << 1));
    }
};

struct fixed {
    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, std::unsigned_integral T>
    static bool encode(Iterator& it, End end, T n) {
        std::size_t i = 0;

        while(it != end) {
            *it++ = n & 0xff;

            if(++i >= sizeof(T)) {
                return true;
            }

            n >>= 8;
        }

        return false; // Error: output overflow.
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, std::signed_integral T>
    static bool encode(Iterator& it, End end, T n) {
        return encode(it, end, static_cast<std::make_unsigned_t<T>>(n));
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End>
    static bool encode(Iterator& it, End end, float n) {
        return encode(it, end, bit_cast<uint32_t>(n));
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End>
    static bool encode(Iterator& it, End end, double n) {
        return encode(it, end, bit_cast<uint64_t>(n));
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End, std::unsigned_integral T>
    static bool decode(Iterator& it, End end, T& n) {
        std::size_t shift = 0;
        n = 0;
        
        while(it != end) {
            n |= static_cast<T>(*it++) << shift;

            if((shift += 8) >= sizeof(T) * 8) {
                return true;
            }
        }

        return false; // Error: input underflow.
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End, std::signed_integral T>
    static bool decode(Iterator& it, End end, T& n) {
        return decode(it, end, reinterpret_cast<std::make_unsigned_t<T>&>(n));
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End>
    static bool decode(Iterator& it, End end, float& n) {
        uint32_t u;
        auto ret = decode(it, end, u);
        n = bit_cast<float>(u);
        return ret;
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End>
    static bool decode(Iterator& it, End end, double& n) {
        uint64_t u;
        auto ret = decode(it, end, u);
        n = bit_cast<double>(u);
        return ret;
    }

    template <typename T>
    constexpr static std::size_t size_bytes(T) {
        return sizeof(T);
    }
};

struct buffer {
    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, typename T>
    static bool encode(Iterator& it, End end, T& n) {
        for(auto b : n) {
            if(it == end) {
                return false; // Error: output overflow.
            }

            *it++ = b;
        }

        return true;
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End, typename T>
    static bool decode(Iterator& it, End end, T& n) {
        //if constexpr (std::is_same_v<Iterator, End>) {
        if constexpr (!std::is_same_v<T, std::vector<uint8_t>> || std::is_same_v<Iterator, End>) {
            n = {it, end};
            it += n.size();

        } else {
            using IT = std::common_iterator<Iterator, End>;

            n = T {IT {it}, IT {end}};
            it += n.size();
        }

        return true;
    }

    template <typename T>
    constexpr static std::size_t size_bytes(T& n) {
        return n.size();
    }
};
