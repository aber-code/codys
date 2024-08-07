#pragma once

#include <codys/Concepts.hpp>
#include <codys/Quantity.hpp>
#include <codys/tuple_utilities.hpp>

#include <units/math.h>

#include <fmt/format.h>
#include <fmt/compile.h>

#include <span>
#include <tuple>

namespace codys
{

template <typename Expression, class Quantities = typename Expression::depends_on, std::size_t N = std::tuple_size_v<typename Expression::depends_on>>
concept SystemExpression = requires(Expression expr, std::span<const double, N> in)
{
    { expr.template evaluate<Quantities, N>(in) } -> std::same_as<double>;
};

template <std::size_t N>
constexpr std::string_view toView(const std::array<char, N>& arr)
{
    return std::string_view(arr.begin(), arr.end());
}

template <SystemExpression Lhs, SystemExpression Rhs> requires
    std::is_same_v<typename Lhs::Unit, typename Rhs::Unit>
struct Add
{
    using depends_on =
    to_unique_tuple_t<tuple_cat_t<typename Lhs::depends_on, typename Rhs::depends_on>>;
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

template <SystemExpression Lhs, SystemExpression Rhs>
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
    to_unique_tuple_t<tuple_cat_t<typename Lhs::depends_on, typename Rhs::depends_on>>;
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

template <SystemExpression Lhs, SystemExpression Rhs>
constexpr auto operator-(Lhs /*lhs*/,
                         Rhs /*rhs*/)
{
    return Substract<Lhs, Rhs>{};
}

template <class Lhs, class Rhs>
struct Multiply
{
    using depends_on =
    to_unique_tuple_t<tuple_cat_t<typename Lhs::depends_on, typename Rhs::depends_on>>;
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

template <SystemExpression Lhs, SystemExpression Rhs>
constexpr auto operator*(Lhs /*lhs*/,
                         Rhs /*rhs*/)
{
    return Multiply<Lhs, Rhs>{};
}

template <class Lhs, class Rhs>
struct Divide
{
    using depends_on =
    to_unique_tuple_t<tuple_cat_t<typename Lhs::depends_on, typename Rhs::depends_on>>;
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

template <SystemExpression Lhs, SystemExpression Rhs>
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

template <SystemExpression Lhs>
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

template <SystemExpression Lhs>
constexpr auto cos(Lhs /*lhs*/)
{
    return Cosinus<Lhs>{};
}

} // namespace codys