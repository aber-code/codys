#pragma once

#include <codys/Concepts.hpp>
#include <codys/Derivative.hpp>
#include <codys/Distinct_Tuple.hpp>
#include <codys/Unique_Tuple.hpp>
#include <codys/tuple_like.hpp>

#include <array>
#include <algorithm>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

namespace codys
{

namespace detail
{

template <TypeIndexedList SystemType, TypeIndexedList ControlsType>
struct StateSpaceSystemIndex
{
    constexpr static auto stateSize = std::tuple_size<typename
        SystemType::UnderlyingType>{};
    constexpr static auto controlSize = std::tuple_size<typename
        ControlsType::UnderlyingType>{};
    constexpr static auto size = stateSize + controlSize;

    using UnderlyingType = decltype(std::tuple_cat(
        std::declval<typename SystemType::UnderlyingType>(),
        std::declval<typename ControlsType::UnderlyingType>()
        ));

    template <class WantedStateType> requires SystemStateFor<
        WantedStateType, SystemType>
    static constexpr std::size_t idx_of()
    {
        return SystemType::template idx_of<WantedStateType>();
    }

    template <class WantedControlType> requires SystemStateFor<
        WantedControlType, ControlsType>
    static constexpr std::size_t idx_of()
    {
        return ControlsType::template idx_of<WantedControlType>() + stateSize;
    }
};

template <typename... States>
consteval System<States...> to_system(std::tuple<States...>)
{
    return {};
}

template <std::size_t N>
constexpr std::string_view toView(const std::array<char, N>& arr)
{
    return std::string_view(arr.begin(), arr.end());
}

// creadits to https://stackoverflow.com/a/42774523/7172556
template <typename Type, std::size_t... sizes>
constexpr auto concatenate(const std::array<Type, sizes>&... arrays)
{
    std::array<Type, (sizes + ...)> result;
    std::size_t index{};

    ((std::copy_n(arrays.begin(), sizes, result.begin() + index), index +=
      sizes), ...);

    return result;
}

template <std::size_t N, std::size_t... Idx>
constexpr auto span_to_tuple_helper(const std::span<const double, N> values,
                                    std::index_sequence<Idx...>)
{
    return std::make_tuple(values[Idx]...);
}

template <std::size_t N>
constexpr auto span_to_tuple(const std::span<const double, N> values)
{
    return span_to_tuple_helper(values, std::make_index_sequence<N>{});
}

} // namespace detail

template <typename... DerivativeEquation>
constexpr auto getStatesDefinedBy(
    const std::tuple<DerivativeEquation...>& /*derivatives*/)
{
    return std::tuple<typename DerivativeEquation::Operand...>{};
}

template <typename T>
concept defines_dot = requires(T t)
{
    t.make_dot();
};

template <defines_dot StateSpaceType>
using states_defined_by_t = std::remove_cvref_t<decltype(getStatesDefinedBy(
    StateSpaceType::make_dot()
    ))>;

template <tuple_like StateSpaceType>
using states_of_t = std::remove_cvref_t<decltype(getStatesDefinedBy(
    std::declval<StateSpaceType>()
    ))>;

template <typename StateSpaceType>
using system_states_t = std::remove_cvref_t<decltype(to_system(
    getStatesDefinedBy(StateSpaceType::make_dot())
    ))>;

template <typename... DerivativeEquation>
constexpr auto getStateDependenciesDefinedBy(
    const std::tuple<DerivativeEquation...>& /*derivatives*/)
{
    using depends_on =
        to_unique_tuple_t<decltype(std::tuple_cat(
            std::declval<typename DerivativeEquation::depends_on>()...
            ))>;

    return depends_on{};
}

template <typename... DerivativeEquation>
constexpr auto getControlsDefinedBy(
    const std::tuple<DerivativeEquation...>& derivatives)
{
    using states = std::remove_cvref_t<decltype(getStatesDefinedBy(derivatives)
    )>;
    using dependants = std::remove_cvref_t<decltype(
        getStateDependenciesDefinedBy(derivatives))>;
    return distinct_tuple_of<dependants, states>{};
}

template <typename StateSpaceType>
using system_controls_t = std::remove_cvref_t<decltype(to_system(
    getControlsDefinedBy(StateSpaceType::make_dot())
    ))>;

// TODO AB 2023-12-22: Move requires into some form of concept for Controls/States when feature "merge systems" is stable
template <TypeIndexedList SystemType, TypeIndexedList ControlsType,
    DerivativeSystemOf<SystemType> StateSpaceType> requires are_distinct<
    typename SystemType::UnderlyingType, typename ControlsType::UnderlyingType>
struct StateSpaceSystem
{
    constexpr static auto stateSize = SystemType::size;
    constexpr static auto stateIndices = std::make_index_sequence<stateSize>{};
    constexpr static auto derivativeFunctions = StateSpaceType::make_dot();
    constexpr static std::size_t derivativeFunctionsSize = std::tuple_size<
        decltype(derivativeFunctions)>{};
    constexpr static auto controlSize = ControlsType::size;

    constexpr static void evaluate(
        std::span<const double, stateSize + controlSize> statesIn,
        std::span<double, stateSize> derivativeValuesOut)
    {

        detail::for_each_in(
            stateIndices, derivativeFunctions,
            [statesIn, derivativeValuesOut]<typename T0>(
            auto /*idx*/, T0 derivative) {
                constexpr auto outIdx = SystemType::template idx_of<typename
                    std::remove_cvref_t<T0>::Operand>();
                derivativeValuesOut[outIdx] =
                    derivative.template evaluate<detail::StateSpaceSystemIndex<
                        SystemType, ControlsType>>(
                        statesIn
                        );
            }
            );
    }

    static std::string format_values(
        std::span<const double, stateSize + controlSize> statesIn)
    {
        std::array<double, stateSize> derivativeValuesOut{};
        std::ranges::fill(derivativeValuesOut, 0.0);
        evaluate(statesIn, derivativeValuesOut);
        auto states2 = std::tuple_cat(detail::span_to_tuple(statesIn),
                                      detail::span_to_tuple(
                                          std::span<const double, stateSize>(
                                              derivativeValuesOut
                                              )
                                          )
            );

        //auto states = std::tuple_cat( typename SystemType::UnderlyingType{}, typename ControlsType::UnderlyingType{}, derivativeFunctions);
        return std::apply([](auto... quantities) {
                              constexpr static auto fmt_string = std::apply(
                                  [](auto... derivs) {
                                      return detail::concatenate(
                                          (derivs.template format_in<detail::StateSpaceSystemIndex<SystemType, ControlsType>>())...
                                          );
                                  }, derivativeFunctions
                                  );
                              return fmt::format(
                                  toView(fmt_string), quantities...
                                  );
                          }, states2
            );
    }

    static std::string format()
    {
        constexpr auto states = std::tuple_cat(
            typename SystemType::UnderlyingType{},
            typename ControlsType::UnderlyingType{}, derivativeFunctions
            );
        return std::apply([](auto... quantities) {
                              constexpr static auto fmt_string = std::apply(
                                  [](auto... derivs) {
                                      return detail::concatenate(
                                          (derivs.template format_in<
                                              detail::StateSpaceSystemIndex<
                                                  SystemType, ControlsType>>())...
                                          );
                                  }, derivativeFunctions
                                  );
                              return fmt::format(
                                  toView(fmt_string), quantities...
                                  );
                          }, states
            );
    }
};

template <typename StateSpaceType>
using StateSpaceSystemOf = StateSpaceSystem<
    system_states_t<StateSpaceType>, system_controls_t<StateSpaceType>,
    StateSpaceType>;

template <typename T, typename U>
struct expression_of
{
};

template <typename T, typename... Derivatives>
struct expression_of<T, std::tuple<Derivatives...>>
{
    using tupleType = std::tuple<Derivatives...>;
    using operands = states_of_t<tupleType>;
    constexpr static auto index = get_idx<T, tupleType>();

    template <typename U = T> requires detail::Contains<T, operands>::value
    constexpr static auto from(tupleType derivatives)
    {
        return std::tuple<typename std::remove_cvref_t<decltype(std::get<index>(derivatives))>::Expression>{};
    }

    template <typename U = T>
    constexpr static std::tuple<> from(tupleType)
    {
        return {};
    }
};

template <typename T, typename... Derivatives>
constexpr auto expression_of_from(std::tuple<Derivatives...> derivatives)
{
    return expression_of<T, std::tuple<Derivatives...>>::from(derivatives);
}

template <typename... Expressions>
constexpr auto combineExpression(std::tuple<Expressions...> expr)
{
    return (std::get<Expressions>(expr) + ...);
}

template <typename Operand, typename... DerivativeSystem>
constexpr auto combineExpressionFor()
{
    return combineExpression(
        std::tuple_cat(
            expression_of_from<Operand>(DerivativeSystem::make_dot())...
            )
        );
}

template <typename... System, typename... Operands>
constexpr auto combineExpressionsFor(std::tuple<Operands...>)
{
    return std::make_tuple(
        dot<Operands>(combineExpressionFor<Operands, System...>())...
        );
}

template <typename... System>
struct combine
{
    using combinedOperands = to_unique_tuple_t<tuple_cat_t<states_defined_by_t<
        System>...>>;

    constexpr static auto make_dot()
    {
        return combineExpressionsFor<System...>(combinedOperands{});
    }
};

} // namespace codys