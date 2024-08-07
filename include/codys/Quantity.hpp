#pragma once

#include "Concepts.hpp"

#include <codys/tuple_utilities.hpp>

#include <fmt/format.h>
#include <fmt/std.h>
#include <fmt/compile.h>

#include <algorithm>
#include <array>
#include <tuple>
#include <span>
#include <string_view>

namespace codys {

template <std::size_t N>
struct StringLiteral
{
    using const_iterator = typename std::array<char, N>::const_iterator;

    constexpr StringLiteral(const char (&str)[N]) // NOLINT[hicpp-explicit-conversions, cppcoreguidelines-avoid-c-arrays]
        : value{} 
    {
        static_assert(N >= 1);
        std::copy_n(std::begin(str), N, value.begin());
    }

    [[nodiscard]] constexpr std::string_view toStringView() const
    {
        return std::string_view(value.data(), N-1);
    }

    std::array<char, N> value; // NOLINT [misc-non-private-member-variables-in-classes]
};

template <typename Tag, typename Unit_, StringLiteral symbol = "">
struct Quantity {
    using Unit = Unit_;
    using depends_on = std::tuple<Quantity>;

    template <class SystemType, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return arr[get_idx<Quantity, SystemType>()];
    }

    template <class SystemType>
    constexpr static auto format_in() {
        constexpr auto index = get_idx<Quantity, SystemType>();
        constexpr auto compiled = FMT_COMPILE("{{{}}}");
        constexpr auto size = fmt::formatted_size(compiled, index);
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, index);
        return result;
    }
};


template<typename value_, typename Unit_>
struct ScalarValue
{
    using depends_on = std::tuple<>;
    using Unit = Unit_;
    constexpr static double value = static_cast<double>(value_::num) / static_cast<double>(value_::den);

    template <class SystemType, std::size_t N> 
    constexpr static double evaluate(std::span<const double, N> /*arr*/) {
        return value;
    }

    template <class SystemType>
    constexpr static auto format_in() {
        if constexpr (value_::den == 1) {
            constexpr auto compiled = FMT_COMPILE("{}");
            constexpr auto size = fmt::formatted_size(compiled, value_::num);
            auto result = std::array<char, size>();
            fmt::format_to(result.data(), compiled, value_::num);
            return result;
        } else {
            constexpr auto compiled = FMT_COMPILE("({}/{})");
            constexpr auto size = fmt::formatted_size(compiled, value_::num, value_::den);
            auto result = std::array<char, size>();
            fmt::format_to(result.data(), compiled, value_::num, value_::den);
            return result;
        }
    }
};


} // namespace codys

template <typename Tag, typename Unit_, ::codys::StringLiteral symbol>
struct fmt::formatter<::codys::Quantity<Tag, Unit_, symbol>>
{
    template <typename ParseContext>
    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(const ::codys::Quantity<Tag, Unit_, symbol>& /*quantity*/, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "{}(t)", 
            symbol.toStringView()
        );
    }
};