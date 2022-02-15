#include <catch2/catch.hpp>

#include <array>
#include <iostream>
#include <span>


#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>
#include <units/isq/si/length.h>

#include <codys.hpp>

using Position = State<class Position_, units::isq::si::length<units::isq::si::metre>>;
using Velocity = State<class Vel_, units::isq::si::speed<units::isq::si::metre_per_second>>;
using Acceleration = State<class Acc_, units::isq::si::acceleration<units::isq::si::metre_per_second_sq>>;

using BasicMotions = System<Position, Velocity>;
struct StateSpace {
    constexpr static auto make_dot() {
        constexpr auto e1 = dot<Velocity>(Velocity{});
        constexpr auto e2 = dot<Position>(Velocity{});
        return boost::hana::make_tuple(e1, e2);
    }
};

TEST_CASE("System is avaluated", "[StateSpace]")
{
  constexpr std::array arr{1.0, 2.0};

  std::array out{1.0, 2.0};

  StateSpaceSystem<BasicMotions, StateSpace>::evaluate(arr, out);

  REQUIRE(out[0] == Approx(2.0));
  REQUIRE(out[1] == Approx(2.0));
}
