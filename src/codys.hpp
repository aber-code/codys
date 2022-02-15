#pragma once

#include <algorithm>
#include <array>
#include <boost/hana.hpp>
#include <boost/hana/concat.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
#include <concepts>
#include <iostream>
#include <iterator>
#include <span>
#include <tuple>
#include <utility>

template <typename T, typename SystemType, std::size_t N>
concept ExpressionFunction = requires(T x, SystemType t,
                                      std::array<double, N> arr) {
    x(t, arr);
};


template <class T, class Tuple>
struct Contains;

template <class T, typename... Ts>
struct Contains<T, boost::hana::tuple<Ts...>> {
    static constexpr bool value = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same<T, Ts>::value...}};

        // You might easily handle duplicate index too (take the last, throw,
        // ...) Here, we select the first one.
        return std::find(a.begin(), a.end(), true) != a.end();
    }();
};

template <class T, class Tuple>
struct Index;

template <class T, typename... Ts>
struct Index<T, boost::hana::tuple<Ts...>> {
    static constexpr std::size_t index = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same<T, Ts>::value...}};

        // You might easily handle duplicate index too (take the last, throw,
        // ...) Here, we select the first one.
        const auto it = std::find(a.begin(), a.end(), true);

        // You might choose other options for not present.

        // As we are in constant expression, we will have compilation error.
        // and not a runtime expection :-)
        if (it == a.end()) throw std::runtime_error("Not present");

        return std::distance(a.begin(), it);
    }();
};

template <class T, class Tuple>
struct TagIndex;

template <class T, typename... Ts>
struct TagIndex<T, boost::hana::tuple<Ts...>> {
    static constexpr int index = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same<T, typename Ts::Operand>::value...}};

        // You might easily handle duplicate index too (take the last, throw,
        // ...) Here, we select the first one.
        const auto it = std::find(a.begin(), a.end(), true);

        // You might choose other options for not present.

        // As we are in constant expression, we will have compilation error.
        // and not a runtime expection :-)
        if (it == a.end())
            return static_cast<decltype(std::distance(a.begin(), it))>(
                sizeof...(Ts));

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

        ret &= ind >= 0 && ind < stateSize;
    });

    return ret;
}

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
concept SystemStateFor = StateType<T> && Contains<T, typename SystemType::StatesType>::value;

template <typename T,typename SystemType>
concept DerivativeSystemOf = StateSystem<SystemType> && all_states_have_derivatives<SystemType, T>();

template <typename Tag_, typename Unit_>
struct State {
    using Tag = Tag_;
    using Unit = Unit_;

    using depends_on = boost::hana::tuple<State<Tag, Unit>>;

    template <StateSystem SystemType, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return arr[SystemType::template idx_of<State<Tag, Unit>>()];
    }
};



template <typename... States>
struct System {
    using StatesType = boost::hana::tuple<States...>;

    template <class StateType> requires SystemStateFor<StateType, System<States...>>
    static constexpr std::size_t idx_of() {
        return Index<StateType, StatesType>::index;
    }
};

template <class Lhs, class Rhs, class Unit_>
struct Plus {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));

        using Unit = Unit_;

    template <class SystemType, std::size_t N> 
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Lhs::template evaluate<SystemType>(arr) +
               Rhs::template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class Unit> 
constexpr auto operator+([[maybe_unused]] State<Lhs, Unit> lhs,
                         [[maybe_unused]] State<Rhs, Unit> rhs) {
    return Plus<State<Lhs, Unit>, State<Rhs, Unit>, Unit>{};
}

template<class Unit>
using derivative_in_time_t = decltype(std::declval<Unit>() / (units::isq::si::time<units::isq::si::second>{}));

template <StateType Operand_, class Expression> 
//requires std::is_same_v<derivative_in_time_t<typename Operand_::Unit>, typename Expression::Unit>
struct Derivative {
    using Operand = Operand_;
    
    template <class SystemType, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Expression::template evaluate<SystemType>(arr);
    }

    using depends_on = typename Expression::depends_on;
};

template <StateType StateName, class Expression>
constexpr auto dot([[maybe_unused]] Expression e) {
    return Derivative<StateName, Expression>();
}

template <StateSystem SystemType, DerivativeSystemOf<SystemType> StateSpaceType>
struct StateSpaceSystem {
    constexpr static std::size_t stateSize = decltype(boost::hana::size(
        std::declval<typename SystemType::StatesType>()))::value;
    constexpr static std::size_t derivativesSize = decltype(boost::hana::size(
        std::declval<StateSpaceType>().make_dot()))::value;
    constexpr static auto stateIndices = std::make_index_sequence<stateSize>{};

    constexpr static void evaluate(
        std::span<const double, stateSize> statesIn,
        std::span<double, stateSize> derivativesOut) {
        constexpr auto dots = StateSpaceType::make_dot();

        boost::hana::for_each(
            stateIndices, [statesIn, derivativesOut, dots](auto idx) {
                derivativesOut[idx] =
                    boost::hana::at(dots, idx).template evaluate<SystemType>(
                        statesIn);
            });
    }
};