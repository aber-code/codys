#include <catch2/catch_test_macros.hpp>

#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>
#include <units/isq/si/length.h>
#include <units/isq/si/acceleration.h>
#include <units/isq/si/area.h>
#include <units/generic/angle.h>
#include <units/generic/dimensionless.h>

#include <codys/Derivative.hpp>
#include <codys/Operators.hpp>
#include <codys/State.hpp>
#include <codys/System.hpp>
#include <codys/StateSpaceSystem.hpp>

#include <array>
#include <cmath>
#include <tuple>
#include <type_traits>

using PositionTag = class Position_;
using PositionUnit = units::isq::si::length<units::isq::si::metre>;
using PositionUnitSq = units::isq::si::area<units::isq::si::square_metre>;
using Dimensionless = units::dimensionless<units::one>;
using Position = codys::State<PositionTag, PositionUnit>;
using VelocityUnit = units::isq::si::speed<units::isq::si::metre_per_second>;
using Velocity = codys::State<class Vel_, VelocityUnit>;
using Rotation = codys::State<class Rot_, units::angle<units::radian, double>>;

using BasicMotions = codys::System<Position, Velocity>;


using PositionX0 = codys::State<class PositionX0_, units::isq::si::length<units::isq::si::metre>>;
using PositionX1 = codys::State<class PositionX1_, units::isq::si::length<units::isq::si::metre>>;
using Acceleration = codys::State<class Acceleration_, units::isq::si::acceleration<units::isq::si::metre_per_second_sq>>;

struct TestSystemMotions
{
  constexpr static auto make_dot()
  {
    constexpr auto dot_velocity = codys::dot<Velocity>(Acceleration{} + Acceleration{});
    constexpr auto dot_pos_x0 = codys::dot<PositionX0>(Velocity{} * codys::cos(Rotation{}));
    constexpr auto dot_pos_x1 = codys::dot<PositionX1>(Velocity{} * codys::sin(Rotation{}));
    return std::make_tuple(dot_velocity, dot_pos_x0, dot_pos_x1);
  }
};

TEST_CASE("System return right state indices", "[System]")
{
  STATIC_REQUIRE(BasicMotions::idx_of<Position>() == 0);
  STATIC_REQUIRE(BasicMotions::idx_of<Velocity>() == 1);
}

TEST_CASE("State gives self as depends_on", "[State]")
{
  STATIC_REQUIRE(std::is_same_v<Position::depends_on, std::tuple<Position>>);
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

TEST_CASE("Chain of Operator Plus yields sum of operands", "[Operator-Chaining]")
{
  static constexpr auto plus = Position{} + Position{} + Position{};
  static constexpr std::array values{ 1.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = plus.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] + values[0] + values[0]);
}

TEST_CASE("Chain of Operator Plus has same unit as operands", "[Operator-Chaining]")
{
  static constexpr auto plus = Position{} + Position{} + Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(plus)::Unit, PositionUnit>);
}

TEST_CASE("Chain of Operator Plus has concatination of denepends on", "[Operator-Chaining]")
{
  static constexpr auto plus = Position{} + Position{} + Position{};
  static constexpr auto ref_dependands = std::tuple<Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(plus)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}

TEST_CASE("Operator Minus yields substraction of operands", "[Operator]")
{
  static constexpr auto minus = Position{} - Position{};
  static constexpr std::array values{ 1.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = minus.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] - values[0]);
}

TEST_CASE("Operator Minus has same unit as operands", "[Operator]")
{
  static constexpr auto minus = Position{} - Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(minus)::Unit, PositionUnit>);
}

TEST_CASE("Operator Minus has concatination of denepends on", "[Operator]")
{
  static constexpr auto minus = Position{} - Position{};
  static constexpr auto ref_dependands = std::tuple<Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(minus)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}

TEST_CASE("Operator Multiply yields multiplication of operands", "[Operator]")
{
  static constexpr auto multiply = Position{} * Position{};
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = multiply.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] * values[0]);
}

TEST_CASE("Operator Multiply has unit resulting from multiplying operands", "[Operator]")
{
  static constexpr auto multiply = Position{} * Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(multiply)::Unit, PositionUnitSq>);
}

TEST_CASE("Operator Multiply has concatination of denepends on", "[Operator]")
{
  static constexpr auto multiply = Position{} * Position{};
  static constexpr auto ref_dependands = std::tuple<Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(multiply)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}

TEST_CASE("Operator Divide yields division of operands", "[Operator]")
{
  static constexpr auto divide = Position{} / Position{};
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = divide.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] / values[0]);
}

TEST_CASE("Operator Divide has unit resulting from dividing operands", "[Operator]")
{
  static constexpr auto divide = Position{} / Position{};
  STATIC_REQUIRE(std::is_same_v<decltype(divide)::Unit, Dimensionless>);
}

TEST_CASE("Operator Divide has concatination of denepends on", "[Operator]")
{
  static constexpr auto divide = Position{} / Position{};
  static constexpr auto ref_dependands = std::tuple<Position>();
  STATIC_REQUIRE(std::is_same_v<decltype(divide)::depends_on, std::remove_cvref_t<decltype(ref_dependands)>>);
}

TEST_CASE("Operator Add to Scalar yields sum of operands", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto displacement = scalar_number * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_sum = displacement + state;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = scalar_sum.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == scalar_number + values[0]);
}

TEST_CASE("Operator Add to Scalar is commutative", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto displacement = scalar_number * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_sum1 = displacement + state;
  static constexpr auto scalar_sum2 = state + displacement;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result1 = scalar_sum1.template evaluate<TestSystem, 1>(values);
  static constexpr auto result2 = scalar_sum2.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result1 == result2);
}

TEST_CASE("Operator Add to Scalar does not change unit", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto displacement = 10 * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_sum = displacement + state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_sum)::Unit, PositionUnit>);
}

TEST_CASE("Operator Add to Scalar does not change dependencies", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto displacement = 10 * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_sum = displacement + state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_sum)::depends_on, decltype(state)::depends_on>);
}

TEST_CASE("Operator Minus with Scalar yields substraction of operands", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto displacement = scalar_number * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_substraction = displacement - state;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = scalar_substraction.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == scalar_number - values[0]);
}

TEST_CASE("Operator Minus with Scalar is commutative", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto displacement = scalar_number * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_substraction1 = displacement - state;
  static constexpr auto scalar_substraction2 = state - displacement;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result1 = scalar_substraction1.template evaluate<TestSystem, 1>(values);
  static constexpr auto result2 = scalar_substraction2.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result1 == -result2);
}

TEST_CASE("Operator Minus with Scalar does not change unit", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto displacement = 10 * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_substraction = displacement - state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_substraction)::Unit, PositionUnit>);
}

TEST_CASE("Operator Minus with Scalar does not change dependencies", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto displacement = 10 * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_substraction = displacement - state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_substraction)::depends_on, decltype(state)::depends_on>);
}

TEST_CASE("Operator Multiply With Scalar yields multiplication of operands", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto time = scalar_number * s;
  static constexpr auto state = Velocity{};
  static constexpr auto scalar_multiply = time * state;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Velocity>;

  static constexpr auto result = scalar_multiply.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == scalar_number * values[0]);
}

TEST_CASE("Operator Multiply With Scalar is commutative", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto time = scalar_number * s;
  static constexpr auto state = Velocity{};
  static constexpr auto scalar_multiply1 = time * state;
  static constexpr auto scalar_multiply2 = state * time;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Velocity>;

  static constexpr auto result1 = scalar_multiply1.template evaluate<TestSystem, 1>(values);
  static constexpr auto result2 = scalar_multiply2.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result1 == result2);
}

TEST_CASE("Operator Multiply With Scalar has unit resulting from multiplying operands", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto time = 10 * s;
  static constexpr auto state = Velocity{};
  static constexpr auto scalar_multiply = time * state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_multiply)::Unit, PositionUnit>);
}

TEST_CASE("Operator Multiply With Scalar does not change dependencies", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto time = 10 * s;
  static constexpr auto state = Velocity{};
  static constexpr auto scalar_multiply = time * state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_multiply)::depends_on, decltype(state)::depends_on>);
}

TEST_CASE("Operator Divide by Scalar yields division of operands", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto time = scalar_number * s;
  static constexpr auto state = Position{};
  static constexpr auto scalar_division = state / time;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = scalar_division.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == values[0] / scalar_number);
}

TEST_CASE("Operator Divide by Scalar yields unit from division", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto time = 10 * s;
  static constexpr auto state = Position{};
  static constexpr auto scalar_division = state / time;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_division)::Unit, VelocityUnit>);
}

TEST_CASE("Operator Divide by Scalar does not change dependencies", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto time = 10 * s;
  static constexpr auto state = Position{};
  static constexpr auto scalar_division = state / time;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_division)::depends_on, decltype(state)::depends_on>);
}

TEST_CASE("Operator Divide Scalar by State yields division of operands", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto scalar_number = 10.0;
  static constexpr auto ref_distance = scalar_number * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_division = ref_distance / state;
  static constexpr std::array values{ 2.0 };
  using TestSystem = codys::System<Position>;

  static constexpr auto result = scalar_division.template evaluate<TestSystem, 1>(values);
  STATIC_REQUIRE(result == scalar_number / values[0]);
}

TEST_CASE("Operator Divide Scalar by State yields unit from division", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto ref_distance = 10.0 * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_division = ref_distance / state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_division)::Unit, Dimensionless>);
}

TEST_CASE("Operator Divide Scalar by State does not change dependencies", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto ref_distance = 10.0 * m;
  static constexpr auto state = Position{};
  static constexpr auto scalar_division = ref_distance / state;
  STATIC_REQUIRE(std::is_same_v<decltype(scalar_division)::depends_on, decltype(state)::depends_on>);
}

TEST_CASE("Operator Sinus yields dimensionless unit", "[Operator]")
{
  static constexpr auto state = Rotation{};
  static constexpr auto sinus = sin(state);
  STATIC_REQUIRE(std::is_same_v<decltype(sinus)::Unit, units::dimensionless<units::one, double>>);
}

TEST_CASE("Operator Sinus yields sinus of operand", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto state = Rotation{};
  static constexpr auto sinus = sin(state);
  static constexpr std::array values{ 3.0 };
  using TestSystem = codys::System<Rotation>;

  static const auto result = sinus.template evaluate<TestSystem, 1>(values);
  REQUIRE(result == std::sin(values[0]));
}

TEST_CASE("Operator Cosinus yields dimensionless unit", "[Operator]")
{
  static constexpr auto state = Rotation{};
  static constexpr auto cosinus = cos(state);
  STATIC_REQUIRE(std::is_same_v<decltype(cosinus)::Unit, units::dimensionless<units::one, double>>);
}

TEST_CASE("Operator Cosinus yields cosinus of operand", "[Operator]")
{
  using namespace units::isq::si::references;
  static constexpr auto state = Rotation{};
  static constexpr auto cosinus = cos(state);
  static constexpr std::array values{ 3.0 };
  using TestSystem = codys::System<Rotation>;

  static const auto result = cosinus.template evaluate<TestSystem, 1>(values);
  REQUIRE(result == std::cos(values[0]));
}

TEST_CASE("States defined by make_dot can be computed", "[TypeUtil]")
{
  constexpr auto states = codys::getStatesDefinedBy(TestSystemMotions::make_dot());
  STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(states)>, std::tuple<Velocity, PositionX0, PositionX1>>);
}


TEST_CASE("Dependants defined by make_dot can be computed", "[TypeUtil]")
{
  constexpr auto states = codys::getStateDependenciesDefinedBy(TestSystemMotions::make_dot());
  STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(states)>, std::tuple<Acceleration, Velocity, Rotation>>);
}

TEST_CASE("Controls defined by make_dot can be computed", "[TypeUtil]")
{
  constexpr auto states = codys::getControlsDefinedBy(TestSystemMotions::make_dot());
  STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(states)>, std::tuple<Acceleration, Rotation>>);
}

TEST_CASE("Expression is determined correctly", "[TypeUtil]")
{
  using dot_velocity = std::remove_cvref_t<decltype(codys::dot<Velocity>(Acceleration{} + Acceleration{}))>;
  using derivativeTypes = std::tuple<dot_velocity>;
  STATIC_REQUIRE(std::is_same_v<codys::expression_of<Velocity, derivativeTypes>::tupleType, derivativeTypes>);
  STATIC_REQUIRE(std::is_same_v<codys::expression_of<Velocity, derivativeTypes>::operands, std::tuple<Velocity>>);
  STATIC_REQUIRE(codys::expression_of<Velocity, derivativeTypes>::index == 0);
}

struct TestSystemMotionsVelOnly
{
  constexpr static auto make_dot()
  {
    constexpr auto dot_velocity = codys::dot<Velocity>(Acceleration{} + Acceleration{});
    return std::make_tuple(dot_velocity);
  }
};


struct TestSystemMotionsPosOnly
{
  constexpr static auto make_dot()
  {
    constexpr auto dot_pos_x0 = codys::dot<PositionX0>(Velocity{} * codys::cos(Rotation{}));
    return std::make_tuple(dot_pos_x0);
  }
};

TEST_CASE("Expressions are combined correctly", "[TypeUtil]")
{
  constexpr auto acc = Acceleration{};
  constexpr auto acc2 = Acceleration{} + Acceleration{};
  constexpr auto accCombined = codys::combineExpression(std::make_tuple(acc, acc2));
  constexpr auto acc3 = Acceleration{} + (Acceleration{} + Acceleration{});
  STATIC_REQUIRE(std::is_same_v< std::remove_cvref_t<decltype(accCombined)>, std::remove_cvref_t<decltype(acc3)>>);

  constexpr auto accCombinedWithEmptySet = codys::combineExpression(std::tuple_cat(std::make_tuple(acc2), std::tuple<>{}));
  STATIC_REQUIRE(std::is_same_v< std::remove_cvref_t<decltype(accCombinedWithEmptySet)>, std::remove_cvref_t<decltype(acc2)>>);

  constexpr auto accCombinedOneArgument = codys::combineExpression(std::make_tuple(acc2));
  STATIC_REQUIRE(std::is_same_v< std::remove_cvref_t<decltype(accCombinedOneArgument)>, std::remove_cvref_t<decltype(acc2)>>);
}

TEST_CASE("Systems are combined correctly", "[TypeUtil]")
{
  using CombinedSys = codys::combine<TestSystemMotionsVelOnly, TestSystemMotionsPosOnly>;
  using combinedOperands = CombinedSys::combinedOperands;

  STATIC_REQUIRE(std::tuple_size_v<combinedOperands> == 2);
  [[maybe_unused]] static constexpr auto derivatives = CombinedSys::make_dot();
}