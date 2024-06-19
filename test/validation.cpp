#include <catch2/catch_test_macros.hpp>

#include <array>

#include <units/isq/si/acceleration.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/length.h>
#include <units/generic/angle.h>
#include <units/generic/dimensionless.h>

#include <codys/Derivative.hpp>
#include <codys/Operators.hpp>
#include <codys/State.hpp>
#include <codys/StateSpaceSystem.hpp>
#include <codys/tuple_utilities.hpp>

#include <fmt/format.h>

#include <cmath>
#include <tuple>
#include <type_traits>

using namespace units::isq::si::references;

namespace
{
using Dimensionless = units::dimensionless<units::one>;

using PositionX0 = codys::State<class PositionX0_, units::isq::si::length<units::isq::si::metre>, "x_0">;
using PositionX1 = codys::State<class PositionX1_, units::isq::si::length<units::isq::si::metre>, "x_1">;
using Velocity2 = codys::State<class Velocity2_, units::isq::si::speed<units::isq::si::metre_per_second>, "v">;
using BasicMotions = std::tuple<PositionX0, PositionX1, Velocity2>;

using Acceleration = codys::State<class Acceleration_, units::isq::si::acceleration<units::isq::si::metre_per_second_sq>, "a">;
using Rotation = codys::State<class Rotation_, units::angle<units::radian, double>>;

using drot_ds_unit = std::remove_cvref_t<decltype(std::declval<units::angle<units::radian, double>>() / (1*s))>;
using drot_dds_unit = std::remove_cvref_t<decltype(std::declval<drot_ds_unit>() / (1*s))>;
using RotationalVelocity = codys::State<class RotationalVelocity_, drot_ds_unit>;
using RotationalAcceleration = codys::State<class RotationalAcceleration_, drot_dds_unit>;


using dacc_ds_unit = std::remove_cvref_t<decltype(std::declval<units::isq::si::acceleration<units::isq::si::metre_per_second_sq>>() / (1*s))>;

using PropellerAngle = codys::State<class PropellerAngle_, units::angle<units::radian, double>>;
using PropellerForce = codys::State<class PropellerForce_, dacc_ds_unit>;


struct Motion2D
{
    constexpr static auto make_dot()
    {
    
        constexpr auto dot_pos_x0 = codys::dot<PositionX0>(Velocity2{} * codys::cos(Rotation{}));
        constexpr auto dot_pos_x1 = codys::dot<PositionX1>(Velocity2{} * codys::sin(Rotation{}));
        constexpr auto dot_velocity = codys::dot<Velocity2>(Acceleration{});
        constexpr auto dot_rotation = codys::dot<Rotation>(RotationalVelocity{});
        constexpr auto dot_rotation_vel = codys::dot<RotationalVelocity>(RotationalAcceleration{});
        return std::make_tuple(dot_velocity, dot_pos_x0, dot_pos_x1, dot_rotation, dot_rotation_vel);
    }
};

struct TurnablePropeller
{
    constexpr static auto make_dot()
    {
        constexpr auto dot_acc = codys::dot<Acceleration>(codys::cos(PropellerAngle{}) * PropellerForce{});
    
        return std::make_tuple(dot_acc);
    }
};

struct Motion2DAdvanced
{
    constexpr static auto make_dot()
    {
        return codys::combine<Motion2D,TurnablePropeller>::make_dot();
    }
};
} // namespace 

TEST_CASE("RealSystem is evaluated correctly", "[RealSystem]")
{
    using Sys = codys::StateSpaceSystemOf<Motion2DAdvanced>;

    constexpr std::array statesIn{  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    std::array out{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

    Sys::evaluate(statesIn, out);
    [[maybe_unused]] const auto formatStr = Sys::format_values(statesIn);
}