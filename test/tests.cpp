#include <catch2/catch.hpp>

#include <array>

#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>
#include <units/generic/angle.h>

#include <codys/codys.hpp>

using PositionX0 = codys::State<class PositionX0_, units::isq::si::length<units::isq::si::metre>>;
using PositionX1 = codys::State<class PositionX1_, units::isq::si::length<units::isq::si::metre>>;
using Velocity = codys::State<class Velocity_, units::isq::si::speed<units::isq::si::metre_per_second>>;
using BasicMotions = codys::System<PositionX0, PositionX1, Velocity>;

using Acceleration = codys::State<class Acceleration_, units::isq::si::acceleration<units::isq::si::metre_per_second_sq>>;
using Rotation = codys::State<class Rotation_, units::angle<units::radian, double>>;
using BasicControls = codys::System<Acceleration, Rotation>;

struct StateSpace
{
  constexpr static auto make_dot()
  {
    constexpr auto dot_velocity = codys::dot<Velocity>(Acceleration{} + Acceleration{});
    constexpr auto dot_pos_x0 = codys::dot<PositionX0>(Velocity{} * codys::cos(Rotation{}));
    constexpr auto dot_pos_x1 = codys::dot<PositionX1>(Velocity{} * codys::sin(Rotation{}));
    return boost::hana::make_tuple(dot_velocity, dot_pos_x0, dot_pos_x1);
  }
};

TEST_CASE("StateSpaceSystem is evaluated correctly", "[StateSpace]")
{
  constexpr std::array statesIn{ 1.0, 1.0, 2.0, 5.0, 0.0};

  std::array out{ 0.0, 0.0, 0.0 };

  codys::StateSpaceSystem<BasicMotions, BasicControls, StateSpace>::evaluate(statesIn, out);

  constexpr std::array expectedOutput{2.0, 0.0, 10.0};
  REQUIRE(out[0] == Approx(expectedOutput[0]));
  REQUIRE(out[1] == Approx(expectedOutput[1]));
  REQUIRE(out[2] == Approx(expectedOutput[2]));
}
