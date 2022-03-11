#include <catch2/catch.hpp>

#include <array>

#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>

#include <boost/hana/tuple.hpp>

#include <codys/codys.hpp>

using PositionTag = class Position_;
using PositionUnit = units::isq::si::length<units::isq::si::metre>;
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
  static constexpr auto ref = boost::hana::tuple<Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(Position::depends_on), decltype(ref)>);
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

TEST_CASE("Operator Plus has same unit as operands", "[Operator]")
{
  static constexpr auto plus = Position{} + Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(plus)::Unit, PositionUnit>);
}

TEST_CASE("Operator Plus has concatination of denepends on", "[Operator]")
{
  static constexpr auto plus = Position{} + Position{};
  static constexpr auto ref_dependands = boost::hana::tuple<Position,Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(plus.depends_on), decltype(ref_dependands)>);
}