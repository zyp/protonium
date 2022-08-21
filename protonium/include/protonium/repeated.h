#pragma once

template <typename T, typename StorageClass, bool Packed>
struct repeated {
    StorageClass::template container<T> data;

    operator std::span<T>() {
        return data;
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End>
    bool decode(Iterator& it, End end) requires Packed {
        while(it != end) {
            auto& element = data.emplace_back();
            if(!element.decode(it, end)) {
                return false;
            }
        }
        return true;
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End>
    bool decode(Iterator& it, End end) requires (!Packed) {
        auto& element = data.emplace_back();
        return element.decode(it, end);
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End>
    bool encode(Iterator& it, End end) const requires Packed {
        for(auto& element : data) {
            if(!element.encode(it, end)) {
                return false;
            }
        }
        return true;
    }

    const auto& elements_to_encode() const requires (!Packed) {
        return data;
    }

    constexpr bool is_present() const {
        return !data.empty();
    }

    constexpr std::size_t size_bytes() const requires Packed {
        std::size_t sum = 0;

        for(auto& element : data) {
            sum += element.size_bytes();
        }

        return sum;
    }
};
