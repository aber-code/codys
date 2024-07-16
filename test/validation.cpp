#include "units/isq/si/angular_acceleration.h"
#include "units/isq/si/angular_velocity.h"

#include <catch2/catch_test_macros.hpp>

#include <array>

#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>
#include <units/generic/angle.h>
#include <units/generic/dimensionless.h>

#include <codys/Derivative.hpp>
#include <codys/Operators.hpp>
#include <codys/Quantity.hpp>
#include <codys/StateSpaceSystem.hpp>
#include <codys/tuple_utilities.hpp>

#include <fmt/format.h>

#include <cmath>
#include <iostream>
#include <tuple>
#include <type_traits>

using namespace units::isq::si::references;
using namespace units::isq::si;
using namespace units;

namespace codys
{

using PositionX0 = Quantity<class PositionX0_, length<metre>, "x_0">;
using PositionX1 = Quantity<class PositionX1_, length<metre>, "x_1">;
using Velocity = Quantity<class Velocity2_, speed<metre_per_second>, "v">;
using Yaw = Quantity<class Yaw_, angle<radian, double>, "\\Phi">;
using Rotation = Quantity<class Rotation_, angular_velocity<radian_per_second>, "\\omega">;
using PropellerAngle = Quantity<class PropellerAngle_, angle<radian, double>, "\\Psi">;
using EOT = Quantity<class PropellerForce_, dimensionless<one>, "EOT">;

using p1 = ScalarValue<std::ratio<1, 2>, acceleration<metre_per_second_sq>>;
using p2 = ScalarValue<std::ratio<1, 3>, angular_acceleration<radian_per_second_sq>>;
using hundret = ScalarValue<std::ratio<100>, dimensionless<one>>;

struct DenebMotion
{
    constexpr static auto make_dot()
    {
        constexpr auto dot_pos_x0 = codys::dot<PositionX0>(Velocity{} * codys::cos(Yaw{}));
        constexpr auto dot_pos_x1 = codys::dot<PositionX1>(Velocity{} * codys::sin(Yaw{}));
        constexpr auto dot_velocity = codys::dot<Velocity>(p1{} * codys::cos(PropellerAngle{}) * (EOT{} / hundret{}));
        constexpr auto dot_yaw = codys::dot<Yaw>(Rotation{});
        constexpr auto dot_rotation = codys::dot<Rotation>(p2{} * codys::sin(PropellerAngle{}) * (EOT{} / hundret{}));

        return std::make_tuple(dot_pos_x0, dot_pos_x1, dot_yaw, dot_velocity, dot_rotation);
    }
};


TEST_CASE("RealSystem is evaluated correctly", "[RealSystem]")
{
    using Sys = StateSpaceSystemOf<DenebMotion>;

    constexpr std::array statesIn{  0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 50.0 };

    std::array out{ 0.0, 0.0, 0.0, 0.0, 0.0};

    Sys::evaluate(statesIn, out);
    std::cout << Sys::format_values(statesIn);
    std::cout << "\n------------\n";
    std::cout << Sys::format();
}

}