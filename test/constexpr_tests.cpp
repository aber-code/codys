#include <catch2/catch.hpp>

#include <array>

#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>
#include <units/isq/si/area.h>
#include <units/generic/dimensionless.h>

#include <boost/hana/tuple.hpp>

#include <codys/codys.hpp>

using PositionTag = class Position_;
using PositionUnit = units::isq::si::length<units::isq::si::metre>;
using PositionUnitSq = units::isq::si::area<units::isq::si::square_metre>;
using Dimensionless = units::dimensionless<units::one>;
using Position = codys::State<PositionTag, PositionUnit>;
using Velocity = codys::State<class Vel_, units::isq::si::speed<units::isq::si::metre_per_second>>;

using BasicMotions = codys::System<Position, Velocity>;

TEST_CASE("System return right state indices", "[System]")
{
  STATIC_REQUIRE(BasicMotions::idx_of<Position>() == 0);
  STATIC_REQUIRE(BasicMotions::idx_of<Velocity>() == 1);
}

TEST_CASE("State gives self as depends_on", "[State]")
{
  STATIC_REQUIRE(std::is_same_v<Position::depends_on, boost::hana::tuple<Position>>);
}

TEST_CASE("State has right unit as type", "[State]")
{
  STATIC_REQUIRE(std::is_same_v<Position::Unit, PositionUnit>);
}

TEST_CASE("Derivative has right operand as type", "[Derivative]") 
{
  static constexpr auto derivative = codys::dot<Position>(Velocity{});
  STATIC_REQUIRE(std::is_same_v<decltype(derivative)::Operand, Position>);
}

TEST_CASE("Operator Plus yields sum of operands", "[Operator]") 
{
  static constexpr auto plus = Position{} + Position{};
  static constexpr std::array values{1.0};
  using TestSystem = codys::System<Position>;

  static constexpr auto result = plus.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] + values[0]);
}

TEST_CASE("Operator Plus has same unit as operands", "[Operator]")
{
  static constexpr auto plus = Position{} + Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(plus)::Unit, PositionUnit>);
}

TEST_CASE("Operator Plus has concatination of denepends on", "[Operator]")
{
  static constexpr auto plus = Position{} + Position{};
  static constexpr auto ref_dependands = boost::hana::tuple<Position,Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(plus)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}

TEST_CASE("Operator Minus yields substraction of operands", "[Operator]") 
{
  static constexpr auto minus = Position{} - Position{};
  static constexpr std::array values{1.0};
  using TestSystem = codys::System<Position>;

  static constexpr auto result = minus.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] - values[0]);
}

TEST_CASE("Operator Minus has same unit as operands", "[Operator]")
{
  static constexpr auto minus = Position{} - Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(minus)::Unit, PositionUnit>);
}

TEST_CASE("Operator Minus has concatination of denepends on", "[Operator]")
{
  static constexpr auto minus = Position{} - Position{};
  static constexpr auto ref_dependands = boost::hana::tuple<Position,Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(minus)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}

TEST_CASE("Operator Multiply yields multiplication of operands", "[Operator]") 
{
  static constexpr auto multiply = Position{} * Position{};
  static constexpr std::array values{2.0};
  using TestSystem = codys::System<Position>;

  static constexpr auto result = multiply.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] * values[0]);
}

TEST_CASE("Operator Multiply has unit resulting from multiplying operands", "[Operator]")
{
  static constexpr auto multiply = Position{} * Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(multiply)::Unit, PositionUnitSq>);
}

TEST_CASE("Operator Multiply has concatination of denepends on", "[Operator]")
{
  static constexpr auto multiply = Position{} * Position{};
  static constexpr auto ref_dependands = boost::hana::tuple<Position,Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(multiply)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}

TEST_CASE("Operator Divide yields multiplication of operands", "[Operator]") 
{
  static constexpr auto divide = Position{} / Position{};
  static constexpr std::array values{2.0};
  using TestSystem = codys::System<Position>;

  static constexpr auto result = divide.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] / values[0]);
}

TEST_CASE("Operator Divide has unit resulting from dividing operands", "[Operator]")
{
  static constexpr auto divide = Position{} / Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(divide)::Unit, Dimensionless>);
}

TEST_CASE("Operator Divide has concatination of denepends on", "[Operator]")
{
  static constexpr auto divide = Position{} / Position{};
  static constexpr auto ref_dependands = boost::hana::tuple<Position,Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(divide)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}