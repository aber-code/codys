#pragma once

#include "Concepts.hpp"

#include <tuple>

namespace codys {

namespace detail {

template <class T, class Tuple>
struct Index;

template <class T, typename... Ts>
struct Index<T, std::tuple<Ts...>> {
    static constexpr std::size_t index = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same<T, Ts>::value...}};

        const auto it = std::find(a.begin(), a.end(), true);

        if (it == a.end()) {
            throw std::runtime_error("Not present");
        }

        return std::distance(a.begin(), it);
    }();
};

} // namespace detail

template <PhysicalType... States>
struct System {
    using UnderlyingType = std::tuple<States...>;

    constexpr static std::size_t size = std::tuple_size<UnderlyingType>{};

    template <class WantedStateType> requires SystemStateFor<WantedStateType, System<States...>>
    static constexpr std::size_t idx_of() {
        return detail::Index<WantedStateType, UnderlyingType>::index;
    }
};

} // namespace codys