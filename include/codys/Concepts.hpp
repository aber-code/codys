#pragma once

#include <boost/hana/for_each.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>

namespace codys {

namespace detail {

template <class T, class Tuple>
struct TagIndex;

template <class T, typename... Ts>
struct TagIndex<T, boost::hana::tuple<Ts...>> {
    static constexpr int index = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same<T, typename Ts::Operand>::value...}};

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
    constexpr auto stateSize = decltype(boost::hana::size(states))::value;
    constexpr auto stateIndices = std::make_index_sequence<stateSize>{};
    
    bool ret = true;
    boost::hana::for_each(stateIndices, [&ret](auto idx) {
        constexpr auto ind = TagIndex<
            typename std::remove_cvref_t<decltype(boost::hana::at(states,
                                                                  idx))>,
            std::remove_cvref_t<decltype(StateSpaceType::make_dot())>>::index;

        ret &= (ind >= 0) && (ind < stateSize);
    });

    return ret;
}

template <class T, class Tuple>
struct Contains;

template <class T, typename... Ts>
struct Contains<T, boost::hana::tuple<Ts...>> {
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
    typename T::Tag;
    typename T::Unit;
};

template <typename SystemType>
concept TypeIndexedList = requires(SystemType sys, typename SystemType::UnderlyingType states) {
    typename SystemType::UnderlyingType;

   {sys.template idx_of<std::remove_cvref_t<decltype(boost::hana::at(states,boost::hana::int_c<0>))>>()} -> std::same_as<std::size_t>;
};

template <typename T, typename SystemType>
concept SystemStateFor = PhysicalType<T> && detail::Contains<T, typename SystemType::UnderlyingType>::value;

template<typename Expression, typename Operand>
concept TimeDerivativeOf = std::is_same_v<detail::derivative_in_time_t<typename Operand::Unit>, typename Expression::Unit>;

template <typename T, typename SystemType>
concept DerivativeSystemOf = TypeIndexedList<SystemType> && detail::all_states_have_derivatives<SystemType, T>();

} // namespace codys