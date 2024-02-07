#pragma once

#include <codys/Concepts.hpp>
#include <codys/Derivative.hpp>
#include <codys/Distinct_Tuple.hpp>
#include <codys/Unique_Tuple.hpp>

#include <span>
#include <tuple>
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

    template<typename U = T> requires detail::Contains<T,operands>::value
    constexpr static auto from(tupleType derivatives)
    {
        return std::make_tuple(std::get<index>(derivatives).expression);
    }

    template<typename U = T> 
    constexpr static std::tuple<> from(tupleType)
    {
        return {};
    }
};

template<typename T, typename... Derivatives>
constexpr auto expression_of_from(std::tuple<Derivatives...> derivatives)
{
    return expression_of<T, std::tuple<Derivatives...>>::from(derivatives);
}

template<typename... Expressions>
constexpr auto combineExpression(std::tuple<Expressions...> expr)
{
    return (std::get<Expressions>(expr) + ...);
}

template<typename Operand, typename... DerivativeSystem>
constexpr auto combineExpressionFor()
{
    return combineExpression(std::tuple_cat(expression_of_from<Operand>(DerivativeSystem::make_dot())...  ) );
}

template<typename SystemLhs, typename SystemRhs, typename... Operands>
constexpr auto combineExpressionsFor(std::tuple<Operands...>)
{
    return std::make_tuple(dot<Operands>(combineExpressionFor<Operands, SystemLhs, SystemRhs>())...);
}

template<typename SystemLhs, typename SystemRhs>
struct combine{

    using combinedOperands = unique_tuple_t< combined_t< states_defined_by_t<SystemLhs>, states_defined_by_t<SystemRhs> > >;
    
    constexpr static auto make_dot()
    {
        return combineExpressionsFor<SystemLhs, SystemRhs>(combinedOperands{});
    }
};

} // namespace codys