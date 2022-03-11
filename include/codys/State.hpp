#pragma once

#include "Concepts.hpp"

#include <boost/hana/tuple.hpp>

#include <span>

namespace codys {

template <typename Tag, typename Unit_>
struct State {
    using Unit = Unit_;

    static constexpr auto depends_on = boost::hana::tuple<State<Tag, Unit>>();

    template <class SystemType, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return arr[SystemType::template idx_of<State<Tag, Unit>>()];
    }
};

} // namespace codys