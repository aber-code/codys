#include <catch2/catch_test_macros.hpp>

#include <array>

#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>
#include <units/generic/angle.h>

#include <codys/Derivative.hpp>
#include <codys/Operators.hpp>
#include <codys/State.hpp>
#include <codys/StateSpaceSystem.hpp>
#include <codys/tuple_utilities.hpp>

#include <cmath>
#include <tuple>
#include <type_traits>
#include <string>

using PositionX0 = codys::State<class PositionX0_, units::isq::si::length<units::isq::si::metre>, "x_0">;
using PositionX1 = codys::State<class PositionX1_, units::isq::si::length<units::isq::si::metre>, "x_1">;
using Velocity = codys::State<class Velocity_, units::isq::si::speed<units::isq::si::metre_per_second> , "v">;
using BasicMotions = std::tuple<PositionX0, PositionX1, Velocity>;

using Acceleration = codys::State<class Acceleration_, units::isq::si::acceleration<units::isq::si::metre_per_second_sq>, "a">;
using Rotation = codys::State<class Rotation_, units::angle<units::radian, double>>;
using BasicControls = std::tuple<Acceleration, Rotation>;

using namespace units::isq::si::references;
using dacc_ds_unit = std::remove_cvref_t<decltype(std::declval<units::isq::si::acceleration<units::isq::si::metre_per_second_sq>>() / (1*s))>;
using PropellerForce = codys::State<class PropellerForce_, dacc_ds_unit>;

using per_second_sq_unit = std::remove_cvref_t<decltype(1 / (1*s) / (1*s))>;
constexpr auto forward_water_resitance = (Velocity{}) * codys::ScalarValue<std::ratio<-1>, per_second_sq_unit >{};


struct Motion2D
{
  constexpr static auto make_dot()
  {
    constexpr auto dot_velocity = codys::dot<Velocity>(Acceleration{} + Acceleration{});
    constexpr auto dot_pos_x0 = codys::dot<PositionX0>(Velocity{} * codys::cos(Rotation{}));
    constexpr auto dot_pos_x1 = codys::dot<PositionX1>(Velocity{} * codys::sin(Rotation{}));
    return std::make_tuple(dot_velocity, dot_pos_x0, dot_pos_x1);
  }
};

struct SimplePropeller
{
  constexpr static auto make_dot()
  {
    constexpr auto dot_acc = codys::dot<Acceleration>(PropellerForce{});
    
    return std::make_tuple(dot_acc);
  }
};

struct ForwardWaterResistance
{
  constexpr static auto make_dot()
  {
    constexpr auto dot_acc = codys::dot<Acceleration>(forward_water_resitance);
    
    return std::make_tuple(dot_acc);
  }
};

struct Motion2DAdvanced
{

  constexpr static auto make_dot()
  {
    return codys::combine<Motion2D,SimplePropeller,ForwardWaterResistance>::make_dot();
  }
};

TEST_CASE("StateSpaceSystem is evaluated correctly", "[StateSpaceSystem]")
{
  // determined to PositionX0, PositionX1, Velocity, Acceleration, Rotation 
  constexpr std::array statesIn{ 1.0, 1.0, 2.0, 5.0, 0.0};

  std::array out{ 0.0, 0.0, 0.0 };

  codys::StateSpaceSystem<BasicMotions, BasicControls, Motion2D>::evaluate(statesIn, out);
  // determined to PositionX0, PositionX1, Velocity
  constexpr std::array expectedOutput{2.0, 0.0, 10.0};
  constexpr auto checkTol = 1e-06;
  REQUIRE(std::abs(out[0] - expectedOutput[0]) < checkTol);
  REQUIRE(std::abs(out[1] - expectedOutput[1]) < checkTol);
  REQUIRE(std::abs(out[2] - expectedOutput[2]) < checkTol);
}

TEST_CASE("StateSpaceSystemOf is evaluated correctly", "[StateSpaceSystemOf]")
{
  // a bit confusing, since we lose the control over the order of states and controls. In real application this will not matter if everything is within the system
  // TODO AB 09.01.2024: Find a way to uncouple the tests and/or make them more expressive

  // determined to Velocity, PositionX0, PositionX1, Acceleration, Rotation, PropellerForce
  constexpr std::array statesIn{ 2.0, 0.0, 0.0, 3.0, 0.0, 5.0};

  std::array out{ 0.0, 0.0, 0.0, 0.0};
  codys::StateSpaceSystemOf<Motion2DAdvanced>::evaluate(statesIn, out);

  // determined to Velocity, PositionX0, PositionX1, Acceleration
  constexpr std::array expectedOutput{6.0, 2.0, 0.0, 3.0};
  constexpr auto checkTol = 1e-06;
  REQUIRE(std::abs(out[0] - expectedOutput[0]) < checkTol);
  REQUIRE(std::abs(out[1] - expectedOutput[1]) < checkTol);
  REQUIRE(std::abs(out[2] - expectedOutput[2]) < checkTol);
  REQUIRE(std::abs(out[3] - expectedOutput[3]) < checkTol);
}

struct SimpleSystem
{
    constexpr static auto make_dot()
    {
        constexpr auto dot_acc = codys::dot<Velocity>(Acceleration{} + Acceleration{});
        constexpr auto dot_pos = codys::dot<PositionX0>(Velocity{});
    
        return std::make_tuple(dot_acc, dot_pos);
    }
};

TEST_CASE("Formatted String contains State Symbols", "[SystemFormat]")
{
  using Sys = codys::StateSpaceSystemOf<SimpleSystem>;
  const auto formatStr = Sys::format();
  REQUIRE(formatStr.find("v(t)") != std::string::npos);
  REQUIRE(formatStr.find("a(t)") != std::string::npos);
  REQUIRE(formatStr.find("x_0(t)") != std::string::npos);
}

TEST_CASE("Formatted Value String contains State Symbols", "[SystemFormat]")
{
  using Sys = codys::StateSpaceSystemOf<SimpleSystem>;
  constexpr std::array states{1.0, 2.0, 3.0};
  const auto formatStr = Sys::format_values(states);
    using namespace std::literals::string_literals;
  REQUIRE(formatStr == "6 = 3 + 3;\n1 = 1;\n"s);
}
