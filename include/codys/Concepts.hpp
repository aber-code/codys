#pragma once

#include <algorithm>
#include <span>
#include <tuple>

namespace codys {

namespace detail {

template<typename Tuple, typename Func, std::size_t... idxs>
constexpr void for_each_in(std::integer_sequence<std::size_t, idxs...> indices, Tuple tup, Func func)
{
    (func(idxs, std::get<idxs>(tup)),...);
}

template <class T, class Tuple>
struct TagIndex;

template <class T, typename... Ts>
struct TagIndex<T, std::tuple<Ts...>> {
    static constexpr int index = []() {
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
    constexpr auto states = typename SystemType::UnderlyingType{};
    constexpr std::size_t stateSize = std::tuple_size<decltype(states)>{};
    constexpr auto stateIndices = std::make_index_sequence<stateSize>{};
    
    bool ret = true;
    for_each_in(stateIndices, states, [&ret](auto idx, auto state) {
        constexpr auto ind = TagIndex<
            typename std::remove_cvref_t<decltype(state)>,
            std::remove_cvref_t<decltype(StateSpaceType::make_dot())>>::index;

        ret &= (ind >= 0) && (ind < stateSize);
    });

    return ret;
}

template <class T, class Tuple>
struct Contains;

template <class T, typename... Ts>
struct Contains<T, std::tuple<Ts...>> {
    static constexpr bool value = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same<T, Ts>::value...}};

        return std::find(a.begin(), a.end(), true) != a.end();
    }();
};

template<class Unit>
using derivative_in_time_t = decltype(std::declval<Unit>() / (units::isq::si::time<units::isq::si::second>{}));

} // namespace detail

template <typename T>
concept PhysicalType = requires {
    typename T::Unit;
};

template <typename T>
concept PhysicalValue = requires (T t) {
    t.number();
};

template <typename T, class System>
concept PhysicalEquation = requires(T eval, std::span<const double> arr) {
    typename T::Unit;

    eval.template evaluate<System>(arr);
};


template <typename SystemType>
concept TypeIndexedList = requires(SystemType sys, typename SystemType::UnderlyingType states) {
    typename SystemType::UnderlyingType;
   sys.size;
   {sys.template idx_of<std::remove_cvref_t<decltype(std::get<0>(states))>>()} -> std::same_as<std::size_t>;
};

template <typename T, typename SystemType>
concept SystemStateFor = PhysicalType<T> && detail::Contains<T, typename SystemType::UnderlyingType>::value;

template<typename Expression, typename Operand>
concept TimeDerivativeOf = std::is_same_v<detail::derivative_in_time_t<typename Operand::Unit>, typename Expression::Unit>;

template <typename T, typename SystemType>
concept DerivativeSystemOf = TypeIndexedList<SystemType> && detail::all_states_have_derivatives<SystemType, T>();

} // namespace codys