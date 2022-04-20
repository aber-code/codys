#pragma once

#include "Concepts.hpp"
#include "State.hpp"

#include <boost/hana/concat.hpp>

#include <span>
 
namespace codys {

template <class Lhs, class Rhs, class Unit_>
struct Add {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = Unit_;

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) +
               rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class Unit> 
constexpr auto operator+([[maybe_unused]] State<Lhs, Unit> lhs,
                         [[maybe_unused]] State<Rhs, Unit> rhs) {
    return Add<State<Lhs, Unit>, State<Rhs, Unit>, Unit>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs, class UnitRhs>
struct AddScalar {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() + std::declval<UnitRhs>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() + rhs.template evaluate<SystemType>(arr);
    }
};

template <PhysicalValue Scalar, class Rhs, class UnitRhs> 
constexpr auto operator+([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return AddScalar<Scalar, State<Rhs, UnitRhs>, UnitRhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Lhs, class UnitLhs> 
constexpr auto operator+([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] Scalar rhs) {
    return AddScalar<Scalar, State<Lhs, UnitLhs>, UnitLhs>{rhs, lhs};
}

template <class Lhs, class Rhs, class Unit_>
struct Substract {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = Unit_;

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) -
               rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class Unit> 
constexpr auto operator-([[maybe_unused]] State<Lhs, Unit> lhs,
                         [[maybe_unused]] State<Rhs, Unit> rhs) {
    return Substract<State<Lhs, Unit>, State<Rhs, Unit>, Unit>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs, class UnitRhs>
struct SubstractScalarFromLeft {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() - std::declval<UnitRhs>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() - rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class UnitLhs, PhysicalValue Scalar>
struct SubstractScalarFromRight {
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(std::declval<UnitLhs>() - std::declval<Scalar>());

    Lhs lhs;
    Scalar rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) - rhs.number();
    }
};

template <PhysicalValue Scalar, class Rhs, class UnitRhs> 
constexpr auto operator-([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return SubstractScalarFromLeft<Scalar, State<Rhs, UnitRhs>, UnitRhs>{lhs, rhs};
}

template <class Lhs, class UnitLhs, PhysicalValue Scalar> 
constexpr auto operator-([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] Scalar rhs) {
    return SubstractScalarFromRight<State<Lhs, UnitLhs>, UnitLhs, Scalar>{lhs, rhs};
}

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs>
struct Multiply {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = decltype(std::declval<UnitLhs>() * std::declval<UnitRhs>());

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) *
               rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs> 
constexpr auto operator*([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return Multiply<State<Lhs, UnitLhs>, State<Rhs, UnitRhs>, UnitLhs, UnitRhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs, class UnitRhs>
struct MultiplyScalar {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() * std::declval<UnitRhs>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() * rhs.template evaluate<SystemType>(arr);
    }
};

template <PhysicalValue Scalar, class Rhs, class UnitRhs> 
constexpr auto operator*([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return MultiplyScalar<Scalar, State<Rhs, UnitRhs>, UnitRhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Lhs, class UnitLhs> 
constexpr auto operator*([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] Scalar rhs) {
    return MultiplyScalar<Scalar, State<Lhs, UnitLhs>, UnitLhs>{rhs, lhs};
}

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs>
struct Divide {
    using depends_on =
        decltype(boost::hana::concat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = decltype(std::declval<UnitLhs>() / std::declval<UnitRhs>());

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) /
               rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class Rhs, class UnitLhs, class UnitRhs> 
constexpr auto operator/([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return Divide<State<Lhs, UnitLhs>, State<Rhs, UnitRhs>, UnitLhs, UnitRhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs, class UnitRhs>
struct DivideScalarFromLeft {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() / std::declval<UnitRhs>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() / rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, class UnitLhs, PhysicalValue Scalar>
struct DivideScalarFromRight {
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(std::declval<UnitLhs>() / std::declval<Scalar>());

    Lhs lhs;
    Scalar rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) / rhs.number();
    }
};

template <PhysicalValue Scalar, class Rhs, class UnitRhs> 
constexpr auto operator/([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] State<Rhs, UnitRhs> rhs) {
    return DivideScalarFromLeft<Scalar, State<Rhs, UnitRhs>, UnitRhs>{lhs, rhs};
}

template <class Lhs, class UnitLhs, PhysicalValue Scalar> 
constexpr auto operator/([[maybe_unused]] State<Lhs, UnitLhs> lhs,
                         [[maybe_unused]] Scalar rhs) {
    return DivideScalarFromRight<State<Lhs, UnitLhs>, UnitLhs, Scalar>{lhs, rhs};
}

} // namespace codys