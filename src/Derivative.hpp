#pragma once

#include "Concepts.hpp"

#include <units/isq/si/time.h>

#include <span>

namespace detail {

template<class Unit>
using derivative_in_time_t = decltype(std::declval<Unit>() / (units::isq::si::time<units::isq::si::second>{}));

} // namespace detail

template <StateType Operand_, class Expression> 
//requires std::is_same_v<derivative_in_time_t<typename Operand_::Unit>, typename Expression::Unit>
struct Derivative {
    using Operand = Operand_;
    
    template <class SystemType, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Expression::template evaluate<SystemType>(arr);
    }

    using depends_on = typename Expression::depends_on;
};

template <StateType StateName, class Expression>
constexpr auto dot([[maybe_unused]] Expression e) {
    return Derivative<StateName, Expression>();
}