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

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N>
    [[nodiscard]] constexpr double evaluate(
        std::span<const double, N> arr) const
    {
        return lhs.template evaluate<SystemType>(arr) +
               rhs.template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    constexpr static auto format_in(Add add)
    {
        constexpr auto fmt_string_lhs = add.lhs.template format_in<SystemType>(
            add.lhs
            );
        constexpr auto fmt_string_rhs = add.rhs.template format_in<SystemType>(
            add.rhs
            );
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
constexpr auto operator+(Lhs lhs,
                         Rhs rhs)
{
    return Add<Lhs, Rhs>{lhs, rhs};
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

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const
    {
        return lhs.template evaluate<SystemType>(arr) -
               rhs.template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    constexpr static auto format_in(Substract sub)
    {
        constexpr auto fmt_string_lhs = sub.lhs.template format_in<SystemType>(
            sub.lhs
            );
        constexpr auto fmt_string_rhs = sub.rhs.template format_in<SystemType>(
            sub.rhs
            );
        constexpr auto compiled = FMT_COMPILE("{} - {}");
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
constexpr auto operator-([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Rhs rhs)
{
    return Substract<Lhs, Rhs>{lhs, rhs};
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

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const
    {
        return lhs.template evaluate<SystemType>(arr) *
               rhs.template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    constexpr static auto format_in(Multiply mult)
    {
        constexpr auto fmt_string_lhs = mult.lhs.template format_in<SystemType>(
            mult.lhs
            );
        constexpr auto fmt_string_rhs = mult.rhs.template format_in<SystemType>(
            mult.rhs
            );
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
constexpr auto operator*([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Rhs rhs)
{
    return Multiply<Lhs, Rhs>{lhs, rhs};
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

    Lhs lhs;
    Rhs rhs;

    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const
    {
        return lhs.template evaluate<SystemType>(arr) /
               rhs.template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    constexpr static auto format_in(Divide div)
    {
        constexpr auto fmt_string_lhs = div.lhs.template format_in<SystemType>(
            div.lhs
            );
        constexpr auto fmt_string_rhs = div.rhs.template format_in<SystemType>(
            div.rhs
            );
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
constexpr auto operator/([[maybe_unused]] Lhs lhs,
                         [[maybe_unused]] Rhs rhs)
{
    return Divide<Lhs, Rhs>{lhs, rhs};
}

template <class Lhs>
struct Sinus
{
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(sin(std::declval<typename Lhs::Unit>()));

    Lhs lhs;

    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const
    {
        return std::sin(lhs.template evaluate<SystemType>(arr));
    }

    template <class SystemType>
    constexpr static auto format_in(Sinus sinus)
    {
        constexpr auto fmt_string_lhs = sinus.lhs.template format_in<
            SystemType>(sinus.lhs);
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
constexpr auto sin([[maybe_unused]] Lhs lhs)
{
    return Sinus<Lhs>{lhs};
}

template <class Lhs>
struct Cosinus
{
    using depends_on = typename Lhs::depends_on;
    using Unit = decltype(cos(std::declval<typename Lhs::Unit>()));

    Lhs lhs;

    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const
    {
        return std::cos(lhs.template evaluate<SystemType>(arr));
    }

    template <class SystemType>
    constexpr static auto format_in(Cosinus cosinus)
    {
        constexpr auto fmt_string_lhs = cosinus.lhs.template format_in<
            SystemType>(cosinus.lhs);
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
constexpr auto cos([[maybe_unused]] Lhs lhs)
{
    return Cosinus<Lhs>{lhs};
}

} // namespace codys