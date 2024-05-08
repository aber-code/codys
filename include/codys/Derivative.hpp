#pragma once

#include "fmt/compile.h"

#include <codys/Concepts.hpp>
#include <codys/Unique_Tuple.hpp>

#include <units/isq/si/time.h>

#include <span>
#include <type_traits>

namespace codys {

template<std::size_t N>
constexpr std::string_view toView2(const std::array<char, N>& arr)
{
    return std::string_view(arr.begin(), arr.end());
}

template <PhysicalType Operand_, TimeDerivativeOf<Operand_> Expression_> 
struct Derivative {
    using Expression = Expression_;
    using depends_on = unique_tuple_t<typename Expression::depends_on>;
    using Operand = Operand_;
    using Unit = detail::derivative_in_time_t<typename Operand::Unit>;  

    Expression expression;
    
    template <class SystemType, std::size_t N>
    constexpr double evaluate(std::span<const double, N> arr) const {
        return expression.template evaluate<SystemType>(arr);
    }

    template <class SystemType>
    constexpr static auto format_in(Derivative deriv) {
        constexpr auto index = SystemType::template idx_of<Operand>() + SystemType::size;
        constexpr auto fmt_string_rhs = deriv.expression.template format_in<SystemType>(deriv.expression);
        constexpr auto compiled = FMT_COMPILE("{{{}}} = {};\n");
        constexpr auto size = fmt::formatted_size(compiled, index, toView2(fmt_string_rhs));
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled , index, toView2(fmt_string_rhs));
        return result;
    }
};

template <PhysicalType StateName, class Expression>
constexpr auto dot([[maybe_unused]] Expression expression) {
    return Derivative<StateName, Expression>{expression};
}

} // namespace codys

template <::codys::PhysicalType Operand_, ::codys::TimeDerivativeOf<Operand_> Expression_> 
struct fmt::formatter<::codys::Derivative<Operand_, Expression_>>
{
    template <typename ParseContext>
    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(const ::codys::Derivative<Operand_, Expression_>& /*deriv*/, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "\\dot({})", 
            Operand_{}
        );
    }
};