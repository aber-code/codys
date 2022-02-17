#include <catch2/catch.hpp>

#include <array>

#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>

#include <codys/codys.hpp>

using Position = codys::State<class Position_, units::isq::si::length<units::isq::si::metre>>;
using Velocity = codys::State<class Vel_, units::isq::si::speed<units::isq::si::metre_per_second>>;

using BasicMotions = codys::System<Position, Velocity>;
struct StateSpace {
    constexpr static auto make_dot() {
        constexpr auto e1 = codys::dot<Velocity>(Velocity{} + Velocity{});
        constexpr auto e2 = codys::dot<Position>(Velocity{});
        return boost::hana::make_tuple(e1, e2);
    }
};

TEST_CASE("StateSpaceSystem is evaluated correctly", "[StateSpace]")
{
  constexpr std::array arr{1.0, 2.0};

  std::array out{0.0, 0.0};

  codys::StateSpaceSystem<BasicMotions, StateSpace>::evaluate(arr, out);

  REQUIRE(out[0] == Approx(2.0));
  REQUIRE(out[1] == Approx(4.0));
}
