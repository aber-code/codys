#pragma once

#include "Concepts.hpp"
#include "State.hpp"

#include <boost/hana/concat.hpp>

#include <span>
 
namespace codys {

template <class Lhs, class Rhs, class Unit_>
struct Plus {
    static constexpr auto depends_on = boost::hana::concat(Lhs::depends_on, Rhs::depends_on);

    using Unit = Unit_;

    template <class SystemType, std::size_t N> 
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Lhs::template evaluate<SystemType>(arr) +
               Rhs::template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class Unit> 
constexpr auto operator+([[maybe_unused]] State<Lhs, Unit> lhs,
                         [[maybe_unused]] State<Rhs, Unit> rhs) {
    return Plus<State<Lhs, Unit>, State<Rhs, Unit>, Unit>{};
}

} // namespace codys