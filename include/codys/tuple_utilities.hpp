#pragma once

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

// Inspired by chatGPT 3.5
// Helper function to check if a type is in a tuple
template <typename T, typename Tuple>
struct is_type_in_tuple;

template <typename T, typename... Types>
struct is_type_in_tuple<T, std::tuple<Types...>> : std::disjunction<std::is_same<T, Types>...> {};

template <typename TupleA, typename TupleB, typename IndexSeq>
struct distinct_tuple_of_impl;

template <typename... TypesA, typename TupleB, std::size_t... Indices>
struct distinct_tuple_of_impl<std::tuple<TypesA...>, TupleB, std::index_sequence<Indices...>> {
    using type = decltype(std::tuple_cat(
        std::conditional_t<is_type_in_tuple<std::tuple_element_t<Indices, std::tuple<TypesA...>>, TupleB>::value,
                           std::tuple<>,
                           std::tuple<std::tuple_element_t<Indices, std::tuple<TypesA...>>>>{}...));
};

} // namespace detail

// replace with std::tuple_like in C++23:
template <typename Tuple>
concept tuple_like = requires(Tuple tuple)
{
    std::get<0>(tuple);
    std::declval<std::tuple_element_t<0, std::remove_cvref_t<Tuple>>>();
    std::tuple_size_v<std::remove_cvref_t<Tuple>>;
};

/// ********* Indexing 

template<class T, tuple_like Tuple>
static constexpr auto get_idx()
{
    return detail::Index<T, Tuple>::index;
}


/// ********* Unique Tuple

template<typename ... Ts>
concept is_unique = detail::unique_helper_v<Ts...>;

template<typename ... Ts>
concept is_unique_tuple = detail::unique_helper_v<Ts...>;

template <typename... Ts>
using unique_tuple = typename detail::unique_tuple_helper<std::tuple<>, Ts...>::type;

template<tuple_like Tuple>
using to_unique_tuple_t = typename detail::to_unique_tuple_helper<Tuple>::type;

template <typename... Args>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<Args>()...));


/// ********** Distinct Tuple

template <typename Lhs, typename Rhs>
concept are_distinct = std::is_same_v<to_unique_tuple_t<tuple_cat_t<Lhs,Rhs>>, tuple_cat_t<to_unique_tuple_t<Lhs>, to_unique_tuple_t<Rhs>>>;

template <tuple_like TupleA, tuple_like TupleB>
using distinct_tuple_of = typename detail::distinct_tuple_of_impl<TupleA, TupleB, std::make_index_sequence<std::tuple_size_v<TupleA>>>::type;

} // namespace codys