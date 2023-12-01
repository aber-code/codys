#pragma once

#include "Concepts.hpp"
#include "State.hpp"
#include "System.hpp"

#include <units/math.h>

#include <span>
#include <tuple>
 
namespace codys {

template<typename T, class States, std::size_t N>
concept SystemExpression = requires (T t, std::span<const double, N> in)
{
    {t.template evaluate<States,N>(in)} -> std::same_as<double>;
};

template<class... States>
constexpr auto to_system(std::tuple<States...>)
{
    return System<States...>{};
}

template<class Tuple>
using to_system_t = decltype(to_system(std::declval<Tuple>()));

template<typename T>
concept EvaluatableOnIdentity = SystemExpression<T, to_system_t<typename T::depends_on>,to_system_t<typename T::depends_on>::size>;

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs> requires std::is_same_v<typename Lhs::Unit, typename Rhs::Unit>
struct Add {
    using depends_on =
        decltype(std::tuple_cat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = typename Rhs::Unit;

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) +
               rhs.template evaluate<SystemType>(arr);
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs> 
constexpr auto operator+([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Rhs rhs) {
    return Add<Lhs, Rhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs>
struct AddScalar {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() + std::declval<typename Rhs::Unit>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() + rhs.template evaluate<SystemType>(arr);
    }
};

template <PhysicalValue Scalar, EvaluatableOnIdentity Rhs> 
constexpr auto operator+([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] Rhs rhs) {
    return AddScalar<Scalar, Rhs>{lhs, rhs};
}

template <PhysicalValue Scalar, EvaluatableOnIdentity Lhs> 
constexpr auto operator+([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Scalar rhs) {
    return AddScalar<Scalar, Lhs>{rhs, lhs};
}

template <class Lhs, class Rhs> requires std::is_same_v<typename Lhs::Unit, typename Rhs::Unit>
struct Substract {
    using depends_on =
        decltype(std::tuple_cat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = typename Rhs::Unit;

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) -
               rhs.template evaluate<SystemType>(arr);
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs> 
constexpr auto operator-([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Rhs rhs) {
    return Substract<Lhs, Rhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs>
struct SubstractScalarFromLeft {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() - std::declval<typename Rhs::Unit>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() - rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, PhysicalValue Scalar>
struct SubstractScalarFromRight {
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(std::declval<typename Lhs::Unit>() - std::declval<Scalar>());

    Lhs lhs;
    Scalar rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) - rhs.number();
    }
};

template <PhysicalValue Scalar, EvaluatableOnIdentity Rhs> 
constexpr auto operator-([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] Rhs rhs) {
    return SubstractScalarFromLeft<Scalar, Rhs>{lhs, rhs};
}

template <EvaluatableOnIdentity Lhs, PhysicalValue Scalar> 
constexpr auto operator-([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Scalar rhs) {
    return SubstractScalarFromRight<Lhs, Scalar>{lhs, rhs};
}

template <class Lhs, class Rhs>
struct Multiply {
    using depends_on =
        decltype(std::tuple_cat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = decltype(std::declval<typename Lhs::Unit>() * std::declval<typename Rhs::Unit>());

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) *
               rhs.template evaluate<SystemType>(arr);
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs> 
constexpr auto operator*([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Rhs rhs) {
    return Multiply<Lhs, Rhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs>
struct MultiplyScalar {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() * std::declval<typename Rhs::Unit>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() * rhs.template evaluate<SystemType>(arr);
    }
};

template <PhysicalValue Scalar, EvaluatableOnIdentity Rhs> 
constexpr auto operator*([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] Rhs rhs) {
    return MultiplyScalar<Scalar, Rhs>{lhs, rhs};
}

template <PhysicalValue Scalar, EvaluatableOnIdentity Lhs> 
constexpr auto operator*([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Scalar rhs) {
    return MultiplyScalar<Scalar, Lhs>{rhs, lhs};
}

template <class Lhs, class Rhs>
struct Divide {
    using depends_on =
        decltype(std::tuple_cat(std::declval<typename Lhs::depends_on>(),
                                     std::declval<typename Rhs::depends_on>()));
    using Unit = decltype(std::declval<typename Lhs::Unit>() / std::declval<typename Rhs::Unit>());

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) /
               rhs.template evaluate<SystemType>(arr);
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs> 
constexpr auto operator/([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Rhs rhs) {
    return Divide<Lhs, Rhs>{lhs, rhs};
}

template <PhysicalValue Scalar, class Rhs>
struct DivideScalarFromLeft {
    using depends_on = typename Rhs::depends_on;
    using Unit = decltype(std::declval<Scalar>() / std::declval<typename Rhs::Unit>());

    Scalar lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.number() / rhs.template evaluate<SystemType>(arr);
    }
};

template <class Lhs, PhysicalValue Scalar>
struct DivideScalarFromRight {
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(std::declval<typename Lhs::Unit>() / std::declval<Scalar>());

    Lhs lhs;
    Scalar rhs;

    template <class SystemType, std::size_t N> 
    constexpr double evaluate(std::span<const double, N> arr) const {
        return lhs.template evaluate<SystemType>(arr) / rhs.number();
    }
};

template <PhysicalValue Scalar, EvaluatableOnIdentity Rhs> 
constexpr auto operator/([[maybe_unused]] Scalar lhs,
                         [[maybe_unused]] Rhs rhs) {
    return DivideScalarFromLeft<Scalar, Rhs>{lhs, rhs};
}

template <EvaluatableOnIdentity Lhs, PhysicalValue Scalar> 
constexpr auto operator/([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Scalar rhs) {
    return DivideScalarFromRight<Lhs, Scalar>{lhs, rhs};
}

template<class Lhs>
struct Sinus
{
  using depends_on = typename Lhs::depends_on;
  using Unit = decltype(sin(std::declval<typename Lhs::Unit>()));

  Lhs lhs;

  template<class SystemType, std::size_t N>
  constexpr double evaluate(std::span<const double, N> arr) const
  {
    return std::sin(lhs.template evaluate<SystemType>(arr));
  }
};

template<EvaluatableOnIdentity Lhs>
constexpr auto sin([[maybe_unused]] Lhs lhs)
{
  return Sinus<Lhs>{ lhs };
}

template<class Lhs>
struct Cosinus
{
  using depends_on = typename Lhs::depends_on;
  using Unit = decltype(cos(std::declval<typename Lhs::Unit>()));

  Lhs lhs;

  template<class SystemType, std::size_t N>
  constexpr double evaluate(std::span<const double, N> arr) const
  {
    return std::cos(lhs.template evaluate<SystemType>(arr));
  }
};

template<EvaluatableOnIdentity Lhs>
constexpr auto cos([[maybe_unused]] Lhs lhs)
{
  return Cosinus<Lhs>{ lhs };
}

} // namespace codys
