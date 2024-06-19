#pragma once

#include <codys/Concepts.hpp>
#include <codys/Unique_Tuple.hpp>

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

template<class T, tuple_like Tuple>
static constexpr auto get_idx()
{
    return detail::Index<T, Tuple>::index;
}

} // namespace codys