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
    constexpr auto states = typename SystemType::StatesType{};
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

} // namespace detail

template <typename T>
concept StateType = requires {
    typename T::Tag;
    typename T::Unit;
};

template <typename SystemType>
concept StateSystem = requires(SystemType sys, typename SystemType::StatesType states) {
    typename SystemType::StatesType;

   {sys.template idx_of<std::remove_cvref_t<decltype(boost::hana::at(states,boost::hana::int_c<0>))>>()} -> std::same_as<std::size_t>;
};

template <typename T, typename SystemType>
concept SystemStateFor = StateType<T> && detail::Contains<T, typename SystemType::StatesType>::value;

template <typename T,typename SystemType>
concept DerivativeSystemOf = StateSystem<SystemType> && detail::all_states_have_derivatives<SystemType, T>();

} // namespace codys