#pragma once

#include "Concepts.hpp"

#include <tuple>

#include <span>

namespace codys {

template <typename Tag, typename Unit_>
struct State {
    using Unit = Unit_;
    using depends_on = std::tuple<State<Tag, Unit>>;

    template <class SystemType, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return arr[SystemType::template idx_of<State<Tag, Unit>>()];
    }
};

} // namespace codys