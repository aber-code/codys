#pragma once

#include "Concepts.hpp"

#include <boost/hana/tuple.hpp>

#include <span>

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