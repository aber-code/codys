#pragma once

#include "Concepts.hpp"

#include <units/isq/si/time.h>

#include <span>
#include <type_traits>

namespace codys {

template <PhysicalType Operand_, TimeDerivativeOf<Operand_> Expression> 
struct Derivative {
    using depends_on = typename Expression::depends_on;
    using Operand = Operand_;

    Expression expression;
    
    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const {
        return expression.template evaluate<SystemType>(arr);
    }
};

template <PhysicalType StateName, class Expression>
constexpr auto dot([[maybe_unused]] Expression expression) {
    return Derivative<StateName, Expression>{expression};
}

} // namespace codys