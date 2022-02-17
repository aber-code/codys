#pragma once

#include "Concepts.hpp"

#include <boost/hana.hpp>

#include <span>
#include <utility>

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
                constexpr auto outIdx = SystemType::template idx_of<typename std::remove_cvref_t<decltype(boost::hana::at(dots, idx))>::Operand>();
                derivativesOut[outIdx] =
                    boost::hana::at(dots, idx).template evaluate<SystemType>(
                        statesIn);
            });
    }
};