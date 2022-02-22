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
  STATIC_REQUIRE(std::is_same_v<Position::depends_on, boost::hana::tuple<Position>>);
}

TEST_CASE("State has right unit as type", "[State]")
{
  STATIC_REQUIRE(std::is_same_v<Position::Unit, PositionUnit>);
}

TEST_CASE("State has right tag as type", "[State]")
{
  STATIC_REQUIRE(std::is_same_v<Position::Tag, PositionTag>);
}