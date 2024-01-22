#pragma once

#include <codys/Concepts.hpp>
#include <codys/Distinct_Tuple.hpp>
#include <codys/Unique_Tuple.hpp>

#include <span>
#include <type_traits>
#include <utility>

namespace codys {

namespace detail {

template <TypeIndexedList SystemType, TypeIndexedList ControlsType>
struct StateSpaceSystemIndex {
    constexpr static std::size_t stateSize = std::tuple_size<typename SystemType::UnderlyingType>{};

    using UnderlyingType = decltype(std::tuple_cat(
        std::declval<typename SystemType::UnderlyingType>(),
        std::declval<typename ControlsType::UnderlyingType>()));

    template <class WantedStateType> requires SystemStateFor<WantedStateType, SystemType>
    static constexpr std::size_t idx_of() {
        return SystemType::template idx_of<WantedStateType>();
    }

    template <class WantedControlType> requires SystemStateFor<WantedControlType, ControlsType>
    static constexpr std::size_t idx_of() {
        return ControlsType::template idx_of<WantedControlType>() + stateSize;
    }
};

template<typename... States>
consteval System<States...> to_system(std::tuple<States...>)
{
    return {}; 
}

}// namespace detail

template<typename... DerivativeEquation>
constexpr auto getStatesDefinedBy(const std::tuple<DerivativeEquation...>& /*derivatives*/)
{
    return std::tuple<typename DerivativeEquation::Operand ...>{};
}

template<typename T>
concept defines_dot = requires(T t)
{
    t.make_dot();
};

// replace with std::tuple_like in C++23:
template <typename Tuple>
concept tuple_like = requires(Tuple tuple)
{
    std::get<0>(tuple);
    std::declval<std::tuple_element_t<0, std::remove_cvref_t<Tuple>>>();
    std::tuple_size_v<std::remove_cvref_t<Tuple>>;
};

template<defines_dot StateSpaceType> 
using states_defined_by_t = std::remove_cvref_t<decltype(getStatesDefinedBy(StateSpaceType::make_dot()))>;

template<tuple_like StateSpaceType> 
using states_of_t = std::remove_cvref_t<decltype(getStatesDefinedBy(std::declval<StateSpaceType>()))>;

template<typename StateSpaceType>
using system_states_t = std::remove_cvref_t<decltype(to_system(getStatesDefinedBy(StateSpaceType::make_dot())))>;

template<typename... DerivativeEquation>
constexpr auto getStateDependenciesDefinedBy(const std::tuple<DerivativeEquation...>& /*derivatives*/)
{
    using depends_on =
        unique_tuple_t<decltype(std::tuple_cat(std::declval<typename DerivativeEquation::depends_on>()...))>;

    return depends_on{};
}

template<typename... DerivativeEquation>
constexpr auto getControlsDefinedBy(const std::tuple<DerivativeEquation...>& derivatives)
{
    using states = std::remove_cvref_t<decltype(getStatesDefinedBy(derivatives))>;
    using dependants = std::remove_cvref_t<decltype(getStateDependenciesDefinedBy(derivatives))>;
    return distinct_tuple_of<dependants, states>{};
}

template<typename StateSpaceType>
using system_controls_t = std::remove_cvref_t<decltype(to_system(getControlsDefinedBy(StateSpaceType::make_dot())))>;

// TODO AB 2023-12-22: Move requires into some form of concept for Controls/States when feature "merge systems" is stable
template <TypeIndexedList SystemType, TypeIndexedList ControlsType, DerivativeSystemOf<SystemType> StateSpaceType> requires are_distinct<typename SystemType::UnderlyingType, typename ControlsType::UnderlyingType>
struct StateSpaceSystem {
    constexpr static auto stateSize = SystemType::size;
    constexpr static auto stateIndices = std::make_index_sequence<stateSize>{};
    constexpr static auto derivativeFunctions = StateSpaceType::make_dot();
    constexpr static std::size_t derivativeFunctionsSize = std::tuple_size<decltype(derivativeFunctions)>{};
    constexpr static auto controlSize = ControlsType::size;

    constexpr static void evaluate(
        std::span<const double, stateSize + controlSize> statesIn,
        std::span<double, stateSize> derivativeValuesOut) {

        detail::for_each_in(
            stateIndices, derivativeFunctions, [statesIn, derivativeValuesOut](auto /*idx*/, auto derivative) {
                constexpr auto outIdx = SystemType::template idx_of<typename std::remove_cvref_t<decltype(derivative)>::Operand>();
                derivativeValuesOut[outIdx] =
                    derivative.template evaluate<detail::StateSpaceSystemIndex<SystemType, ControlsType>>(
                        statesIn);
            });
    }
};

template<typename StateSpaceType>
using StateSpaceSystemOf = StateSpaceSystem<system_states_t<StateSpaceType>, system_controls_t<StateSpaceType>, StateSpaceType>;

template<typename T, typename U>
struct expression_of{};

template<typename T, typename... Derivatives>
struct expression_of<T, std::tuple<Derivatives...>>
{
    using tupleType = std::tuple<Derivatives...>;
    using operands = states_of_t<tupleType>;
    constexpr static auto index = detail::TagIndex<T, tupleType>::index;
    using type = std::conditional_t<
        detail::Contains<T,operands>::value,
        std::tuple<
            typename std::remove_cvref_t<
                        decltype(std::get<index>(std::declval<tupleType>()))
                    >::Expression
        >,
        std::tuple<>
    >;
};

template<typename T, typename Derivatives>
using expression_of_t = expression_of<T, Derivatives>::type;


} // namespace codys