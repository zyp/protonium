#pragma once

//template<typename T, typename Iterator, typename End>
//concept Encodable = requires(T a, Iterator it, End end)
//{
//    { a.encode(it, end) } -> std::convertible_to<bool>;
//};
//
//template<typename T, typename Iterator, typename End>
//concept EncodableElements = requires(T a)
//{
//    { *a.elements_to_encode().begin() } -> Encodable<Iterator, End>;
//};

template<typename T>
concept Encodable = requires(T a)
{
    //{ a.encode(it, end) } -> std::convertible_to<bool>;
    { a.size_bytes() } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept EncodableElements = requires(T a)
{
    { *a.elements_to_encode().begin() } -> Encodable;
};

struct message_base {
    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End, typename T>
    static bool decode_field(Iterator& it, End end, tag_t tag, T& field) {
        if(tag.type() == tag_t::length_delimited) {
            length_t length;
            if(!length.decode(it, end)) {
                return false;
            }

            //auto sub_end = it + length.n;

            //if(sub_end > end) {
            //    return false;
            //}

            //return field.decode(it, sub_end);

            // TODO: Detect and error out when original iterator terminates before length.n.
            std::counted_iterator sub_it {it, static_cast<std::iter_difference_t<Iterator>>(length.n)};

            auto ret = field.decode(sub_it, std::default_sentinel);

            it = sub_it.base();

            return ret;
        }

        return field.decode(it, end);
    }

    template <std::input_iterator Iterator, std::sentinel_for<Iterator> End>
    static bool skip_field(Iterator& it, End end, tag_t tag) {
        std::size_t skip_bytes = 0;

        switch(tag.type()) {
            case tag_t::varint:
                while(it != end) {
                    if(((*it++) & 0x80) == 0) {
                        return true;
                    }
                }
                return false;
            
            case tag_t::length_delimited:
                {
                    length_t length;
                    if(!length.decode(it, end)) {
                        return false;
                    }
                    skip_bytes = length;
                    break;
                }
            
            case tag_t::fixed32:
                skip_bytes = 4;
                break;
            
            case tag_t::fixed64:
                skip_bytes = 4;
                break;
            
            default:
                return false;
        }

        while(skip_bytes--) {
            if(it == end) {
                return false;
            }

            (void)*it++;
        }

        return true;
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, Encodable T>
    static bool encode_field(Iterator& it, End end, tag_t tag, const T& field) {
        if(!field.is_present()) {
            return true;
        }

        if(!tag.encode(it, end)) {
            return false;
        }

        if(tag.type() == tag_t::length_delimited) {
            length_t length {field.size_bytes()};
            if(!length.encode(it, end)) {
                return false;
            }
        }

        return field.encode(it, end);
    }

    template <std::output_iterator<uint8_t> Iterator, std::sentinel_for<Iterator> End, EncodableElements T>
    static bool encode_field(Iterator& it, End end, tag_t tag, const T& field) {
        for(auto& element : field.elements_to_encode()) {
            if(!tag.encode(it, end)) {
                return false;
            }

            if(!element.encode(it, end)) {
                return false;
            }
        }

        return true;
    }

    template <Encodable T>
    constexpr static std::size_t size_field(tag_t tag, const T& field) {
        if(!field.is_present()) {
            return 0;
        }

        if(tag.type() == tag_t::length_delimited) {
            length_t length {field.size_bytes()};
            return tag.size_bytes() + length.size_bytes() + length;
        }

        return tag.size_bytes() + field.size_bytes();
    }

    template <EncodableElements T>
    constexpr static std::size_t size_field(tag_t tag, const T& field) {
        std::size_t sum = 0;

        for(auto& element : field.elements_to_encode()) {
            size_field(tag, element);
        }

        return sum;
    }
};
