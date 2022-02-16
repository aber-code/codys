#include <catch2/catch.hpp>

#include <array>

#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>

#include <codys.hpp>

using Position = State<class Position_, units::isq::si::length<units::isq::si::metre>>;
using Velocity = State<class Vel_, units::isq::si::speed<units::isq::si::metre_per_second>>;

using BasicMotions = System<Position, Velocity>;

TEST_CASE("System return right state indices")
{
  STATIC_REQUIRE(BasicMotions::idx_of<Position>() == 0);
  STATIC_REQUIRE(BasicMotions::idx_of<Velocity>() == 1);
}
