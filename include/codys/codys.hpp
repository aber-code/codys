#pragma once

#include <tuple>

namespace codys {

namespace detail {

template <class T, class Tuple>
struct Index;

template <class T, typename... Ts>
struct Index<T, std::tuple<Ts...>> {
    static constexpr std::size_t index = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same<T, Ts>::value...}};

        const auto it = std::find(a.begin(), a.end(), true);

        if (it == a.end()) {
            throw std::runtime_error("Not present");
        }

        return std::distance(a.begin(), it);
    }();
};

template<typename ...Ts>
struct unique_helper
{
    using value = std::true_type;
};

template<typename T, typename... Ts> requires (sizeof...(Ts) > 0)
struct unique_helper<T, Ts...> 
{
    using value = std::conditional_t<(std::is_same_v<T, Ts> || ...)
                                     , std::false_type
                                     , typename unique_helper<Ts...>::value>;
};

template<typename... Ts>
constexpr bool unique_helper_v = typename unique_helper<Ts...>::value{};

template<typename... Ts>
constexpr bool unique_helper_v<std::tuple<Ts...>> = typename unique_helper<Ts...>::value{};

// credits for unique: https://stackoverflow.com/a/57528226/7172556
template <typename T, typename... Ts>
struct unique_tuple_helper : std::type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct unique_tuple_helper<std::tuple<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...)
                         , unique_tuple_helper<std::tuple<Ts...>, Us...>
                         , unique_tuple_helper<std::tuple<Ts..., U>, Us...>> {};

template<typename T>
struct to_unique_tuple_helper{};

template<typename... Ts>
struct to_unique_tuple_helper<std::tuple<Ts...>>
{
    using type = typename detail::unique_tuple_helper<std::tuple<>, Ts...>::type;
};

// Inspired by chatGPT 3.5
// Helper function to check if a type is in a tuple
template <typename T, typename Tuple>
struct is_type_in_tuple;

template <typename T, typename... Types>
struct is_type_in_tuple<T, std::tuple<Types...>> : std::disjunction<std::is_same<T, Types>...> {};

template <typename TupleA, typename TupleB, typename IndexSeq>
struct distinct_tuple_of_impl;

template <typename... TypesA, typename TupleB, std::size_t... Indices>
struct distinct_tuple_of_impl<std::tuple<TypesA...>, TupleB, std::index_sequence<Indices...>> {
    using type = decltype(std::tuple_cat(
        std::conditional_t<is_type_in_tuple<std::tuple_element_t<Indices, std::tuple<TypesA...>>, TupleB>::value,
                           std::tuple<>,
                           std::tuple<std::tuple_element_t<Indices, std::tuple<TypesA...>>>>{}...));
};

} // namespace detail

// replace with std::tuple_like in C++23:
template <typename Tuple>
constexpr bool empty_tuple = std::tuple_size_v<std::remove_cvref_t<Tuple>> == 0;

// replace with std::tuple_like in C++23:
template <typename Tuple>
concept sized_tuple = requires(Tuple tuple)
{
    std::get<0>(tuple);
    std::declval<std::tuple_element_t<0, std::remove_cvref_t<Tuple>>>();
    std::tuple_size_v<std::remove_cvref_t<Tuple>>;
};

template <typename Tuple>
concept tuple_like = empty_tuple<Tuple> || sized_tuple<Tuple>;


/// ********* Indexing 

template<class T, tuple_like Tuple>
static constexpr auto get_idx()
{
    return detail::Index<T, Tuple>::index;
}


/// ********* Unique Tuple

template<typename ... Ts>
concept is_unique = detail::unique_helper_v<Ts...>;

template<typename ... Ts>
concept is_unique_tuple = detail::unique_helper_v<Ts...>;

template <typename... Ts>
using unique_tuple = typename detail::unique_tuple_helper<std::tuple<>, Ts...>::type;

template<tuple_like Tuple>
using to_unique_tuple_t = typename detail::to_unique_tuple_helper<Tuple>::type;

template <typename... Args>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<Args>()...));


/// ********** Distinct Tuple

template <typename Lhs, typename Rhs>
concept are_distinct = std::is_same_v<to_unique_tuple_t<tuple_cat_t<Lhs,Rhs>>, tuple_cat_t<to_unique_tuple_t<Lhs>, to_unique_tuple_t<Rhs>>>;

template <tuple_like TupleA, tuple_like TupleB>
using distinct_tuple_of = typename detail::distinct_tuple_of_impl<TupleA, TupleB, std::make_index_sequence<std::tuple_size_v<TupleA>>>::type;

} // namespace codys

#include <algorithm>
#include <span>
#include <tuple>
#include <type_traits>

namespace codys {

namespace detail {

template<typename Tuple, typename Func, std::size_t... idxs>
constexpr void tuple_for_each_impl(std::integer_sequence<std::size_t, idxs...> /*indices*/, Tuple tup, Func func)
{
    (func(std::get<idxs>(tup)),...);
}

template<typename Tuple, typename Func>
constexpr void tuple_for_each(Tuple tup, Func func)
{
    tuple_for_each_impl(std::make_index_sequence<std::tuple_size_v<Tuple>>{}, tup, func);
}

template <class T, class Tuple>
struct TagIndex;

template <class T, typename... Ts>
struct TagIndex<T, std::tuple<Ts...>> {
    static constexpr auto index = []() {
        constexpr std::array<bool, sizeof...(Ts)> a{
            {std::is_same_v<T, typename Ts::Operand>...}};

        const auto it = std::find(a.begin(), a.end(), true);
        if (it == a.end()) {
            return static_cast<decltype(std::distance(a.begin(), it))>(
                sizeof...(Ts));
        }

        return std::distance(a.begin(), it);
    }();
};

template <typename States, typename DerivativeSystem>
constexpr bool all_states_have_derivatives() {
    bool ret = true;
    tuple_for_each(States{}, [&ret]<typename StateType>(StateType /*state*/) {
        constexpr auto ind = TagIndex<
            StateType,
            std::remove_cvref_t<decltype(DerivativeSystem::make_dot())>>::index;

        ret &= (ind >= 0) && (ind < std::tuple_size_v<States>);
    });

    return ret;
}

template <class T, class Tuple>
struct Contains;

template <class T, typename... Ts>
struct Contains<T, std::tuple<Ts...>> {
    static constexpr bool value = []() {
        constexpr std::array<bool, sizeof...(Ts)> existenceMask{
            {std::is_same<T, Ts>::value...}};

        return std::find(existenceMask.begin(), existenceMask.end(), true) != existenceMask.end();
    }();
};

template<class Unit>
using derivative_in_time_t = decltype(std::declval<Unit>() / (units::isq::si::time<units::isq::si::second>{}));

} // namespace detail


template<class T, tuple_like Tuple>
static constexpr auto get_operator_idx()
{
    return detail::TagIndex<T, Tuple>::index;
}

template <typename T>
concept PhysicalType = requires {
    typename T::Unit;
};

template <typename List>
concept TypeIndexedList = requires(List sys) {
   {get_idx<std::remove_cvref_t<decltype(std::get<0>(sys))>, List>()} -> std::same_as<std::size_t>;
};

template <typename T, typename Quantities>
concept SystemStateFor = PhysicalType<T> && detail::Contains<T, typename Quantities::UnderlyingType>::value;

template<typename Expression, typename Operand>
concept TimeDerivativeOf = std::is_same_v<detail::derivative_in_time_t<typename Operand::Unit>, typename Expression::Unit>;

template <typename T, typename States>
concept DerivativeSystemOf = TypeIndexedList<States> && detail::all_states_have_derivatives<States, T>();

} // namespace codys

#include "fmt/compile.h"

#include <units/isq/si/time.h>

#include <span>
#include <type_traits>

namespace codys
{

template <std::size_t N>
constexpr std::string_view toView2(const std::array<char, N>& arr)
{
    return std::string_view(arr.begin(), arr.end());
}

template <PhysicalType Operand_, TimeDerivativeOf<Operand_> Expression_>
struct Derivative
{
    using Expression = Expression_;
    using depends_on = to_unique_tuple_t<typename Expression::depends_on>;
    using Operand = Operand_;
    using Unit = detail::derivative_in_time_t<typename Operand::Unit>;

    template <class Quantities, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Expression::template evaluate<Quantities>(arr);
    }

    template <class Quantities>
    constexpr static auto format_in()
    {
        constexpr auto index = get_idx<Operand, Quantities>() +
                               std::tuple_size_v<Quantities>;
        constexpr auto fmt_string_rhs = Expression::template format_in<Quantities>();
        constexpr auto compiled = FMT_COMPILE("{{{}}} = {};\n");
        constexpr auto size = fmt::formatted_size(compiled, index, toView2(fmt_string_rhs));
        auto result = std::array<char, size>();
        fmt::format_to(result.data(), compiled, index, toView2(fmt_string_rhs));
        return result;
    }
};

template <PhysicalType StateName, class Expression>
constexpr auto dot(Expression /*expression*/)
{
    return Derivative<StateName, Expression>{};
}

} // namespace codys

template <::codys::PhysicalType Operand_, ::codys::TimeDerivativeOf<Operand_>
    Expression_>
struct fmt::formatter<::codys::Derivative<Operand_, Expression_>>
{
    template <typename ParseContext>
    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(
        const ::codys::Derivative<Operand_, Expression_>& /*deriv*/,
        FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "\\dot({})",
            Operand_{}
            );
    }
};



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

    template <class Quantities, std::size_t N>
    constexpr static double evaluate(std::span<const double, N> arr) {
        return arr[get_idx<Quantity, Quantities>()];
    }

    template <class Quantities>
    constexpr static auto format_in() {
        constexpr auto index = get_idx<Quantity, Quantities>();
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

    template <class Quantities, std::size_t N> 
    constexpr static double evaluate(std::span<const double, N> /*arr*/) {
        return value;
    }

    template <class Quantities>
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

namespace codys {

template<typename Tuple>
concept set_of_types = is_unique_tuple<Tuple>;

template<set_of_types... set>
using merge_t = to_unique_tuple_t<tuple_cat_t<set...>>;

template<set_of_types Lhs, set_of_types Rhs>
using set_difference_t = distinct_tuple_of<Lhs, Rhs>;

} // namespace codys

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
    using depends_on = merge_t<typename Lhs::depends_on, typename Rhs::depends_on>;
    using Unit = typename Rhs::Unit;

    template <class Quantities, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<Quantities>(arr) +
               Rhs::template evaluate<Quantities>(arr);
    }

    template <class Quantities>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<Quantities>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<Quantities>();
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
    using depends_on = merge_t<typename Lhs::depends_on, typename Rhs::depends_on>;
    using Unit = typename Rhs::Unit;

    template <class Quantities, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<Quantities>(arr) -
               Rhs::template evaluate<Quantities>(arr);
    }

    template <class Quantities>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<Quantities>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<Quantities>();
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
    using depends_on = merge_t<typename Lhs::depends_on, typename Rhs::depends_on>;
    using Unit = decltype(std::declval<typename Lhs::Unit>() * std::declval<
                              typename Rhs::Unit>());

    template <class Quantities, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<Quantities>(arr) *
               Rhs::template evaluate<Quantities>(arr);
    }

    template <class Quantities>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<Quantities>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<Quantities>();
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
    using depends_on = merge_t<typename Lhs::depends_on, typename Rhs::depends_on>;
    using Unit = decltype(std::declval<typename Lhs::Unit>() / std::declval<
                              typename Rhs::Unit>());

    template <class Quantities, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return Lhs::template evaluate<Quantities>(arr) /
               Rhs::template evaluate<Quantities>(arr);
    }

    template <class Quantities>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<Quantities>();
        constexpr auto fmt_string_rhs = Rhs::template format_in<Quantities>();
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

    template <class Quantities, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return std::sin(Lhs::template evaluate<Quantities>(arr));
    }

    template <class Quantities>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<Quantities>();
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

    template <class Quantities, std::size_t N>
    [[nodiscard]] static constexpr double evaluate(std::span<const double, N> arr)
    {
        return std::cos(Lhs::template evaluate<Quantities>(arr));
    }

    template <class Quantities>
    static constexpr auto format_in()
    {
        constexpr auto fmt_string_lhs = Lhs::template format_in<Quantities>();
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



#include <array>
#include <algorithm>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

namespace codys
{

namespace detail
{

template <std::size_t N>
constexpr std::string_view toView(const std::array<char, N>& arr)
{
    return std::string_view(arr.begin(), arr.end());
}

// creadits to https://stackoverflow.com/a/42774523/7172556
template <typename Type, std::size_t... sizes>
constexpr auto concatenate(const std::array<Type, sizes>&... arrays)
{
    std::array<Type, (sizes + ...)> result;
    std::size_t index{};

    ((std::copy_n(arrays.begin(), sizes, result.begin() + index), index +=
      sizes), ...);

    return result;
}

template <std::size_t N, std::size_t... Idx>
constexpr auto span_to_tuple_helper(const std::span<const double, N> values,
                                    std::index_sequence<Idx...>)
{
    return std::make_tuple(values[Idx]...);
}

template <std::size_t N>
constexpr auto span_to_tuple(const std::span<const double, N> values)
{
    return span_to_tuple_helper(values, std::make_index_sequence<N>{});
}

} // namespace detail

template<typename System, typename T>
concept has_idx_of = requires(System)
{
    System::template idx_of<T>();
};

template<class T, typename System> requires has_idx_of<System, T>
static constexpr auto get_idx()
{
    return System::template idx_of<T>();
}

template <typename... DerivativeEquation>
constexpr auto getStatesDefinedBy(
    const std::tuple<DerivativeEquation...>& /*derivatives*/)
{
    return std::tuple<typename DerivativeEquation::Operand...>{};
}

template <typename T>
concept defines_dot = requires(T t)
{
    t.make_dot();
};

template <defines_dot StateSpaceType>
using states_defined_by_t = std::remove_cvref_t<decltype(getStatesDefinedBy(
    StateSpaceType::make_dot()
    ))>;

template <tuple_like StateSpaceType>
using states_of_t = std::remove_cvref_t<decltype(getStatesDefinedBy(
    std::declval<StateSpaceType>()
    ))>;

template <typename StateSpaceType>
using system_states_t = std::remove_cvref_t<decltype(
    getStatesDefinedBy(StateSpaceType::make_dot())
)>;

template <typename... DerivativeEquation>
constexpr auto getStateDependenciesDefinedBy(
    const std::tuple<DerivativeEquation...>& /*derivatives*/)
{
    using depends_on = merge_t<typename DerivativeEquation::depends_on...>;
    return depends_on{};
}

template <typename... DerivativeEquation>
constexpr auto getControlsDefinedBy(
    const std::tuple<DerivativeEquation...>& derivatives)
{
    using states = std::remove_cvref_t<decltype(getStatesDefinedBy(derivatives)
    )>;
    using dependants = std::remove_cvref_t<decltype(
        getStateDependenciesDefinedBy(derivatives))>;
    return set_difference_t<dependants, states>{};
}

template <typename StateSpaceType>
using system_controls_t = std::remove_cvref_t<decltype(
    getControlsDefinedBy(StateSpaceType::make_dot())
    )>;

// TODO AB 2023-12-22: Move requires into some form of concept for Controls/States when feature "merge systems" is stable
template <TypeIndexedList States, TypeIndexedList Controls, DerivativeSystemOf<States> DerivativeSystem> requires are_distinct<States, Controls>
struct StateSpaceSystem
{
    using AllQuantities = tuple_cat_t<States, Controls>;
    constexpr static auto stateSize = std::tuple_size_v<States>;
    constexpr static auto controlSize = std::tuple_size_v<Controls>;

    constexpr static void evaluate(
        std::span<const double, stateSize + controlSize> statesIn,
        std::span<double, stateSize> derivativesOut)
    {
        detail::tuple_for_each(DerivativeSystem::make_dot(),
                            [statesIn, derivativesOut]<typename DerivativeType>(DerivativeType derivative) {
                                constexpr auto outIdx = get_idx<typename DerivativeType::Operand, States>();
                                derivativesOut[outIdx] = derivative.template evaluate<AllQuantities>(statesIn);
                            }
        );
        
    }

    static std::string format_values(
        std::span<const double, stateSize + controlSize> statesIn)
    {
        std::array<double, stateSize> derivativeValuesOut{};
        std::ranges::fill(derivativeValuesOut, 0.0);
        evaluate(statesIn, derivativeValuesOut);
        auto states2 = std::tuple_cat(detail::span_to_tuple(statesIn),
                                      detail::span_to_tuple(
                                          std::span<const double, stateSize>(
                                              derivativeValuesOut
                                              )
                                          )
            );

        return std::apply([](auto... quantities) {
                              constexpr static auto fmt_string = std::apply(
                                  [](auto... derivs) {
                                      return detail::concatenate(
                                          (derivs.template format_in<AllQuantities>())...
                                          );
                                  }, DerivativeSystem::make_dot()
                                  );
                              return fmt::format(
                                  toView(fmt_string), quantities...
                                  );
                          }, states2
            );
    }

    static std::string format()
    {
        constexpr auto states = std::tuple_cat(
            AllQuantities{}, DerivativeSystem::make_dot()
            );
        return std::apply([](auto... quantities) {
                              constexpr static auto fmt_string = std::apply(
                                  [](auto... derivs) {
                                      return detail::concatenate(
                                          (derivs.template format_in<AllQuantities>())...
                                          );
                                  }, DerivativeSystem::make_dot()
                                  );
                              return fmt::format(
                                  toView(fmt_string), quantities...
                                  );
                          }, states
            );
    }
};

template <typename StateSpaceType>
using StateSpaceSystemOf = StateSpaceSystem<
    system_states_t<StateSpaceType>, system_controls_t<StateSpaceType>,
    StateSpaceType>;

template <typename T, typename U>
struct expression_of
{
};

template<std::size_t idx, typename Tuple>
using at_idx_t = std::remove_cvref_t<decltype(std::get<idx>(std::declval<Tuple>()))>;

template <typename T, typename... Derivative>
struct expression_of<T, std::tuple<Derivative...>>
{
    using derivatives = std::tuple<Derivative...>;
    using operands = std::tuple<typename Derivative::Operand...>;

    template <typename U = T> requires detail::Contains<U, operands>::value
    constexpr static auto value()
    {
        constexpr auto index = get_idx<U, operands>();
        return std::tuple<typename at_idx_t<index, derivatives>::Expression>{};
    }

    template <typename U = T>
    constexpr static std::tuple<> value()
    {
        return {};
    }
};

template<typename T, tuple_like Derivatives>
constexpr auto expression_of_v = expression_of<T, Derivatives>::value();

template <typename... Expression>
constexpr auto combineExpressions(std::tuple<Expression...> expr)
{
    return (std::get<Expression>(expr) + ...);
}

template <typename Operand, typename... DerivativeSystem>
constexpr auto combineExpressionsFor()
{
    return combineExpressions(
        std::tuple_cat(
            expression_of_v<Operand, decltype(DerivativeSystem::make_dot())>...
            )
        );
}

template <typename... DerivativeSystem, typename... Operand>
constexpr auto combineDerivatesFor(std::tuple<Operand...>)
{
    return std::make_tuple(
        dot<Operand>(combineExpressionsFor<Operand, DerivativeSystem...>())...
        );
}

template <typename... DerivativeSystem>
struct combine
{
    using combinedOperands = merge_t<states_defined_by_t<DerivativeSystem>...>;

    constexpr static auto make_dot()
    {
        return combineDerivatesFor<DerivativeSystem...>(combinedOperands{});
    }
};

} // namespace codys
