#pragma once

#include <codys/Concepts.hpp>
#include <codys/State.hpp>
#include <codys/System.hpp>
#include <codys/Unique_Tuple.hpp>

#include <units/math.h>

#include <span>
#include <tuple>

namespace codys
{

template <typename T, class States, std::size_t N>
concept SystemExpression = requires(T t, std::span<const double, N> in)
{
    { t.template evaluate<States, N>(in) } -> std::same_as<double>;
};

template <class... States>
constexpr auto to_system(std::tuple<States...>)
{
    return System<States...>{};
}

template <class Tuple>
using to_system_t = decltype(to_system(std::declval<Tuple>()));

template <typename T>
concept EvaluatableOnIdentity = SystemExpression<
    T, to_system_t<typename T::depends_on>, to_system_t<typename
        T::depends_on>::size>;

template <std::size_t N>
constexpr std::string_view toView(const std::array<char, N>& arr)
{
    return std::string_view(arr.begin(), arr.end());
}

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs> requires
    std::is_same_v<typename Lhs::Unit, typename Rhs::Unit>
struct Add
{
    using depends_on =
    to_unique_tuple_t<decltype(std::tuple_cat(
        std::declval<typename Lhs::depends_on>(),
        std::declval<typename Rhs::depends_on>()
        ))>;
    using Unit = typename Rhs::Unit;

    template <class SystemType, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<SystemType>(arr) +
               Rhs::template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<SystemType>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<SystemType>();
        constexpr auto compiled = FMT_COMPILE("{} + {}");
        constexpr auto size = fmt::formatted_size(
            compiled, toView(fmt_string_lhs), toView(fmt_string_rhs)
            );
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, toView(fmt_string_lhs),
                       toView(fmt_string_rhs)
            );
        return result;
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs>
constexpr auto operator+(Lhs /*lhs*/,
                         Rhs /*rhs*/)
{
    return Add<Lhs, Rhs>{};
}

template <class Lhs, class Rhs> requires std::is_same_v<
    typename Lhs::Unit, typename Rhs::Unit>
struct Substract
{
    using depends_on =
    to_unique_tuple_t<decltype(std::tuple_cat(
        std::declval<typename Lhs::depends_on>(),
        std::declval<typename Rhs::depends_on>()
        ))>;
    using Unit = typename Rhs::Unit;

    template <class SystemType, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<SystemType>(arr) -
               Rhs::template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<SystemType>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<SystemType>();
        constexpr auto compiled = FMT_COMPILE("{} - {}");
        constexpr auto size = fmt::formatted_size(compiled, toView(fmt_string_lhs), toView(fmt_string_rhs));
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, toView(fmt_string_lhs), toView(fmt_string_rhs));
        return result;
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs>
constexpr auto operator-(Lhs /*lhs*/,
                         Rhs /*rhs*/)
{
    return Substract<Lhs, Rhs>{};
}

template <class Lhs, class Rhs>
struct Multiply
{
    using depends_on =
    to_unique_tuple_t<decltype(std::tuple_cat(
        std::declval<typename Lhs::depends_on>(),
        std::declval<typename Rhs::depends_on>()
        ))>;
    using Unit = decltype(std::declval<typename Lhs::Unit>() * std::declval<
                              typename Rhs::Unit>());

    template <class SystemType, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<SystemType>(arr) *
               Rhs::template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<SystemType>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<SystemType>();
        constexpr auto compiled = FMT_COMPILE("{} * {}");
        constexpr auto size = fmt::formatted_size(
            compiled, toView(fmt_string_lhs), toView(fmt_string_rhs)
            );
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, toView(fmt_string_lhs),
                       toView(fmt_string_rhs)
            );
        return result;
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs>
constexpr auto operator*(Lhs /*lhs*/,
                         Rhs /*rhs*/)
{
    return Multiply<Lhs, Rhs>{};
}

template <class Lhs, class Rhs>
struct Divide
{
    using depends_on =
    to_unique_tuple_t<decltype(std::tuple_cat(
        std::declval<typename Lhs::depends_on>(),
        std::declval<typename Rhs::depends_on>()
        ))>;
    using Unit = decltype(std::declval<typename Lhs::Unit>() / std::declval<
                              typename Rhs::Unit>());

    template <class SystemType, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<SystemType>(arr) /
               Rhs::template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<SystemType>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<SystemType>();
        constexpr auto compiled = FMT_COMPILE("\\frac({})({})");
        constexpr auto size = fmt::formatted_size(
            compiled, toView(fmt_string_lhs), toView(fmt_string_rhs)
            );
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, toView(fmt_string_lhs),
                       toView(fmt_string_rhs)
            );
        return result;
    }
};

template <EvaluatableOnIdentity Lhs, EvaluatableOnIdentity Rhs>
constexpr auto operator/(Lhs /*lhs*/,
                         Rhs /*rhs*/)
{
    return Divide<Lhs, Rhs>{};
}

template <class Lhs>
struct Sinus
{
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(sin(std::declval<typename Lhs::Unit>()));

    template <class SystemType, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return std::sin(Lhs::template evaluate<SystemType>(arr));
    }

    template <class SystemType>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<SystemType>();
        constexpr auto compiled = FMT_COMPILE("\\sin({})");
        constexpr auto size = fmt::formatted_size(
            compiled, toView(fmt_string_lhs)
            );
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, toView(fmt_string_lhs));
        return result;
    }
};

template <EvaluatableOnIdentity Lhs>
constexpr auto sin(Lhs /*lhs*/)
{
    return Sinus<Lhs>{};
}

template <class Lhs>
struct Cosinus
{
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(cos(std::declval<typename Lhs::Unit>()));

    template <class SystemType, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return std::cos(Lhs::template evaluate<SystemType>(arr));
    }

    template <class SystemType>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<SystemType>();
        constexpr auto compiled = FMT_COMPILE("\\cos({})");
        constexpr auto size = fmt::formatted_size(
            compiled, toView(fmt_string_lhs)
            );
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, toView(fmt_string_lhs));
        return result;
    }
};

template <EvaluatableOnIdentity Lhs>
constexpr auto cos(Lhs /*lhs*/)
{
    return Cosinus<Lhs>{};
}

} // namespace codys