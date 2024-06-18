#pragma once

#include <codys/tuple_like.hpp>

#include <tuple>
#include <type_traits>

namespace codys {

namespace detail {
template<typename ...Ts>
struct unique_helper
{
    using value = std::true_type;
};

template<typename T, typename... Ts> requires (sizeof...(Ts) > 0)
struct unique_helper<T, Ts...> 
{
    using value = std::conditional_t<(std::is_same_v<T, Ts> || ...)
                                     , std::false_type
                                     , typename unique_helper<Ts...>::value>;
};

template<typename... Ts>
constexpr bool unique_helper_v = typename unique_helper<Ts...>::value{};

template<typename... Ts>
constexpr bool unique_helper_v<std::tuple<Ts...>> = typename unique_helper<Ts...>::value{};

// credits for unique: https://stackoverflow.com/a/57528226/7172556
template <typename T, typename... Ts>
struct unique_tuple_helper : std::type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct unique_tuple_helper<std::tuple<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...)
                         , unique_tuple_helper<std::tuple<Ts...>, Us...>
                         , unique_tuple_helper<std::tuple<Ts..., U>, Us...>> {};

template<typename T>
struct to_unique_tuple_helper{};

template<typename... Ts>
struct to_unique_tuple_helper<std::tuple<Ts...>>
{
    using type = typename detail::unique_tuple_helper<std::tuple<>, Ts...>::type;
};

} // namespace detail

template<typename ... Ts>
concept is_unique = detail::unique_helper_v<Ts...>;

template<typename ... Ts>
concept is_unique_tuple = detail::unique_helper_v<Ts...>;

template <typename... Ts>
using unique_tuple = typename detail::unique_tuple_helper<std::tuple<>, Ts...>::type;

template<tuple_like Tuple>
using to_unique_tuple_t = typename detail::to_unique_tuple_helper<Tuple>::type;

template <tuple_like... Args>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<Args>()...));

template <typename Lhs, typename Rhs>
concept are_distinct = std::is_same_v<to_unique_tuple_t<tuple_cat_t<Lhs,Rhs>>, tuple_cat_t<to_unique_tuple_t<Lhs>, to_unique_tuple_t<Rhs>>>;

} // namespace codys