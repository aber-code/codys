#pragma once

#include <codys/Concepts.hpp>
#include <codys/Unique_Tuple.hpp>

#include <span>
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

}// namespace detail

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

} // namespace codys