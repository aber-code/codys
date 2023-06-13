#include <catch2/catch.hpp>

#include <array>

#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>
#include <units/generic/angle.h>

#include <codys/codys.hpp>

using Position = codys::State<class Position_, units::isq::si::length<units::isq::si::metre>>;
using Velocity = codys::State<class Vel_, units::isq::si::speed<units::isq::si::metre_per_second>>;
using Rotation = codys::State<class Rot_, units::angle<units::radian, double>>;
using BasicMotions = codys::System<Position, Velocity>;

using Acceleration = codys::State<class Acc_, units::isq::si::acceleration<units::isq::si::metre_per_second_sq>>;
using BasicControls = codys::System<Acceleration>;

struct StateSpace
{
  constexpr static auto make_dot()
  {
    constexpr auto e1 = codys::dot<Velocity>(Acceleration{} + Acceleration{});
    constexpr auto e2 = codys::dot<Position>(Velocity{});
    return boost::hana::make_tuple(e1, e2);
  }
};

TEST_CASE("StateSpaceSystem is evaluated correctly", "[StateSpace]")
{
  constexpr std::array statesIn{ 1.0, 2.0, 5.0 };

  std::array out{ 0.0, 0.0 };

  codys::StateSpaceSystem<BasicMotions, BasicControls, StateSpace>::evaluate(statesIn, out);

  constexpr auto expectedOutPutAt0 = 2.0;
  constexpr auto expectedOutputAt1 = 10.0;
  REQUIRE(out[0] == Approx(expectedOutPutAt0));
  REQUIRE(out[1] == Approx(expectedOutputAt1));
}
