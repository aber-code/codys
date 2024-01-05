#pragma once

#include <tuple>
#include <type_traits>

namespace codys { 

// Inspired by chatGPT

// Helper function to check if a type is in a tuple
template <typename T, typename Tuple>
struct is_type_in_tuple;

template <typename T, typename... Types>
struct is_type_in_tuple<T, std::tuple<Types...>> : std::disjunction<std::is_same<T, Types>...> {};

// Recursive case for distinct_tuple_of_impl
template <typename TupleA, typename TupleB, typename = std::make_index_sequence<std::tuple_size_v<TupleA>>>
struct distinct_tuple_of_impl;

template <typename... TypesA, typename TupleB, std::size_t... Indices>
struct distinct_tuple_of_impl<std::tuple<TypesA...>, TupleB, std::index_sequence<Indices...>> {
    using type = decltype(std::tuple_cat(
        std::conditional_t<is_type_in_tuple<std::tuple_element_t<Indices, std::tuple<TypesA...>>, TupleB>::value,
                           std::tuple<>,
                           std::tuple<std::tuple_element_t<Indices, std::tuple<TypesA...>>>>{}...));
};

// Base case for distinct_tuple_of
template <typename TupleB>
struct distinct_tuple_of_impl<std::tuple<>, TupleB, std::index_sequence<>> {
    using type = std::tuple<>;
};

// Convenience alias for distinct_tuple_of
template <typename TupleA, typename TupleB>
using distinct_tuple_of = typename distinct_tuple_of_impl<TupleA, TupleB>::type;

} // namespace codys