#pragma once

#include <codys/Concepts.hpp>
#include <codys/Unique_Tuple.hpp>

#include <units/isq/si/time.h>

#include <span>
#include <type_traits>

namespace codys {

template <PhysicalType Operand_, TimeDerivativeOf<Operand_> Expression> 
struct Derivative {
    using depends_on = unique_tuple_t<typename Expression::depends_on>;
    using Operand = Operand_;
    using Unit = detail::derivative_in_time_t<typename Operand::Unit>;

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