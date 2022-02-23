#pragma once

#include "Concepts.hpp"

#include <units/isq/si/time.h>

#include <span>
#include <type_traits>

namespace codys {

template <PhysicalType Operand_, TimeDerivativeOf<Operand_> Expression> 
struct Derivative {
    using Operand = Operand_;
    
    template <class SystemType, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Expression::template evaluate<SystemType>(arr);
    }

    static constexpr auto depends_on = Expression::depends_on;
};

template <PhysicalType StateName, class Expression>
constexpr auto dot([[maybe_unused]] Expression e) {
    return Derivative<StateName, Expression>();
}

} // namespace codys