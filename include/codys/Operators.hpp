#pragma once

#include "Concepts.hpp"
#include "State.hpp"

#include <boost/hana/concat.hpp>

#include <span>
 
namespace codys {

template <class Lhs, class Rhs, class Unit_>
struct Plus {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));

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

template <class Lhs, class Rhs, class Unit_>
struct Minus {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));

    using Unit = Unit_;

    template <class SystemType, std::size_t N> 
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Lhs::template evaluate<SystemType>(arr) -
               Rhs::template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class Unit> 
constexpr auto operator-([[maybe_unused]] State<Lhs, Unit> lhs,
                         [[maybe_unused]] State<Rhs, Unit> rhs) {
    return Minus<State<Lhs, Unit>, State<Rhs, Unit>, Unit>{};
}

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs>
struct Multiply {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));

    using Unit = decltype(std::declval<UnitLhs>() * std::declval<UnitRhs>());

    template <class SystemType, std::size_t N> 
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Lhs::template evaluate<SystemType>(arr) *
               Rhs::template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs> 
constexpr auto operator*([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return Multiply<State<Lhs, UnitLhs>, State<Rhs, UnitRhs>, UnitLhs, UnitRhs>{};
}

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs>
struct Divide {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));

    using Unit = decltype(std::declval<UnitLhs>() / std::declval<UnitRhs>());

    template <class SystemType, std::size_t N> 
    constexpr static double evaluate(std::span<const double, N> arr) {
        return Lhs::template evaluate<SystemType>(arr) /
               Rhs::template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs> 
constexpr auto operator/([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return Divide<State<Lhs, UnitLhs>, State<Rhs, UnitRhs>, UnitLhs, UnitRhs>{};
}

} // namespace codys