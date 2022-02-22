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
    constexpr static std::size_t stateSize = decltype(boost::hana::size(
        std::declval<typename SystemType::UnderlyingType>()))::value;
    constexpr static std::size_t derivativesSize = decltype(boost::hana::size(
        std::declval<StateSpaceType>().make_dot()))::value;
    constexpr static auto stateIndices = std::make_index_sequence<stateSize>{};

    constexpr static std::size_t controlSize = decltype(boost::hana::size(
        std::declval<typename ControlsType::UnderlyingType>()))::value;

    constexpr static void evaluate(
        std::span<const double, stateSize + controlSize> statesIn,
        std::span<double, stateSize> derivativesOut) {
        constexpr auto dots = StateSpaceType::make_dot();

        boost::hana::for_each(
            stateIndices, [statesIn, derivativesOut, dots](auto idx) {
                constexpr auto outIdx = SystemType::template idx_of<typename std::remove_cvref_t<decltype(boost::hana::at(dots, idx))>::Operand>();
                derivativesOut[outIdx] =
                    boost::hana::at(dots, idx).template evaluate<detail::StateSpaceSystemIndex<SystemType, ControlsType>>(
                        statesIn);
            });
    }
};

} // namespace codys