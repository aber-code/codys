#pragma once

#include <codys/tuple_utilities.hpp>

namespace codys {

template<tuple_like Tuple>
concept set_of_types = is_unique_tuple<Tuple>;

} // namespace codys
