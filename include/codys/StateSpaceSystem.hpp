#pragma once

#include "Concepts.hpp"

#include <boost/hana.hpp>

#include <span>
#include <utility>

namespace codys {

namespace detail {

template <TypeIndexedList SystemType, TypeIndexedList ControlsType>
struct StateSpaceSystemIndex {
    constexpr static std::size_t stateSize = decltype(boost::hana::size(
        std::declval<typename SystemType::UnderlyingType>()))::value;

    using UnderlyingType = decltype(boost::hana::concat(
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

template <TypeIndexedList SystemType, TypeIndexedList ControlsType, DerivativeSystemOf<SystemType> StateSpaceType>
struct StateSpaceSystem {
    constexpr static auto stateSize = SystemType::size;
    constexpr static auto stateIndices = std::make_index_sequence<stateSize>{};
    constexpr static auto derivativeFunctions = StateSpaceType::make_dot();
    constexpr static auto derivativeFunctionsSize = decltype(boost::hana::size(derivativeFunctions))::value;
    constexpr static auto controlSize = ControlsType::size;

    constexpr static void evaluate(
        std::span<const double, stateSize + controlSize> statesIn,
        std::span<double, stateSize> derivativeValuesOut) {

        boost::hana::for_each(
            stateIndices, [statesIn, derivativeValuesOut](auto idx) {
                constexpr auto outIdx = SystemType::template idx_of<typename std::remove_cvref_t<decltype(boost::hana::at(derivativeFunctions, idx))>::Operand>();
                derivativeValuesOut[outIdx] =
                    boost::hana::at(derivativeFunctions, idx).template evaluate<detail::StateSpaceSystemIndex<SystemType, ControlsType>>(
                        statesIn);
            });
    }
};

} // namespace codys