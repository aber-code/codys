#pragma once

#include <codys/tuple_utilities.hpp>

#include <algorithm>
#include <span>
#include <tuple>
#include <type_traits>

namespace codys {

namespace detail {

template<typename Tuple, typename Func, std::size_t... idxs>
constexpr void tuple_for_each_impl(std::integer_sequence<std::size_t, idxs...> /*indices*/, Tuple tup, Func func)
{
    (func(std::get<idxs>(tup)),...);
}

template<typename Tuple, typename Func>
constexpr void tuple_for_each(Tuple tup, Func func)
{
    tuple_for_each_impl(std::make_index_sequence<std::tuple_size_v<Tuple>>{}, tup, func);
}

template <class T, class Tuple>
struct TagIndex;

template <class T, typename... Ts>
struct TagIndex<T, std::tuple<Ts...>> {
    static constexpr auto index = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same_v<T, typename Ts::Operand>...}};

        const auto it = std::find(a.begin(), a.end(), true);
        if (it == a.end()) {
            return static_cast<decltype(std::distance(a.begin(), it))>(
                sizeof...(Ts));
        }

        return std::distance(a.begin(), it);
    }();
};

template <typename SystemType, typename StateSpaceType>
constexpr bool all_states_have_derivatives() {
    bool ret = true;
    tuple_for_each(SystemType{}, [&ret]<typename StateType>(StateType /*state*/) {
        constexpr auto ind = TagIndex<
            StateType,
            std::remove_cvref_t<decltype(StateSpaceType::make_dot())>>::index;

        ret &= (ind >= 0) && (ind < std::tuple_size_v<SystemType>);
    });

    return ret;
}

template <class T, class Tuple>
struct Contains;

template <class T, typename... Ts>
struct Contains<T, std::tuple<Ts...>> {
    static constexpr bool value = []() {
        constexpr std::array<bool, sizeof...(Ts)> existenceMask{
            {std::is_same<T, Ts>::value...}};

        return std::find(existenceMask.begin(), existenceMask.end(), true) != existenceMask.end();
    }();
};

template<class Unit>
using derivative_in_time_t = decltype(std::declval<Unit>() / (units::isq::si::time<units::isq::si::second>{}));

} // namespace detail


template<class T, tuple_like Tuple>
static constexpr auto get_operator_idx()
{
    return detail::TagIndex<T, Tuple>::index;
}

template <typename T>
concept PhysicalType = requires {
    typename T::Unit;
};

template <typename SystemType>
concept TypeIndexedList = requires(SystemType sys) {
   {get_idx<std::remove_cvref_t<decltype(std::get<0>(sys))>, SystemType>()} -> std::same_as<std::size_t>;
};

template <typename T, typename SystemType>
concept SystemStateFor = PhysicalType<T> && detail::Contains<T, typename SystemType::UnderlyingType>::value;

template<typename Expression, typename Operand>
concept TimeDerivativeOf = std::is_same_v<detail::derivative_in_time_t<typename Operand::Unit>, typename Expression::Unit>;

template <typename T, typename SystemType>
concept DerivativeSystemOf = TypeIndexedList<SystemType> && detail::all_states_have_derivatives<SystemType, T>();

} // namespace codys