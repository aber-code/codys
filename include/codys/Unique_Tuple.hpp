#pragma once

#include <tuple>
#include <type_traits>

namespace codys {

// credits for unique: https://stackoverflow.com/a/57528226/7172556
template <typename T, typename... Ts>
struct unique : std::type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct unique<std::tuple<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...)
                       , unique<std::tuple<Ts...>, Us...>
                       , unique<std::tuple<Ts..., U>, Us...>> {};

template <typename... Ts>
using unique_tuple = typename unique<std::tuple<>, Ts...>::type;

template<typename T>
struct to_unique{};

template<typename... Ts>
struct to_unique<std::tuple<Ts...>>
{
    using type = unique_tuple<Ts...>;
};

template<typename Tuple>
using unique_tuple_t = to_unique<Tuple>::type;

template <typename... Ts> 
concept is_unique = std::is_same_v<unique_tuple<Ts...>, std::tuple<Ts...>>;

template <typename... Args>
using combined_t = decltype(std::tuple_cat(std::declval<Args>()...));

template <typename Lhs, typename Rhs>
concept are_distinct = std::is_same_v<unique_tuple_t<combined_t<Lhs,Rhs>>, combined_t<unique_tuple_t<Lhs>, unique_tuple_t<Rhs>>>;

} // namespace codys