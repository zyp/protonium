#pragma once

#include <variant>

template <typename... Types>
struct oneof {
    std::variant<std::monostate, Types...> payload;

    template <std::size_t I>
    auto& emplace(auto&& value) {
        return payload.template emplace<I + 1>(value);
    }

    bool is_present(std::size_t i) {
        return payload.index() == (i + 1);
    }

    template <std::size_t I>
    auto& get() {
        if(!is_present(I)) {
            payload.template emplace<I + 1>();
        }

        return std::get<I + 1>(payload);
    }
};

template <std::size_t I, typename Parent>
struct oneof_field {
    Parent& parent;

    oneof_field(Parent& parent) : parent(parent) {}

    template <typename T>
    auto& operator=(const T& value) {
        return parent.template emplace<I>(value);
    }

    auto& get() {
        return parent.template get<I>();
    }

    const auto& get() const {
        return parent.template get<I>();
    }

    auto& operator*() {
        return get();
    }

    const auto& operator*() const {
        return get();
    }

    auto* operator->() {
        return &get();
    }

    const auto* operator->() const {
        return &get();
    }

    operator bool() const {
        return is_present();
    }

    bool decode(auto& it, auto end) {
        return get().decode(it, end);
    }

    bool encode(auto& it, auto end) const {
        return get().encode(it, end);
    }

    constexpr bool is_present() const {
        return parent.is_present(I);
    }

    constexpr std::size_t size_bytes() const {
        return get().size_bytes();
    }
};
