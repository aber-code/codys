#pragma once

#include <codys/Unique_Tuple.hpp>
#include <codys/tuple_like.hpp>

namespace codys {

template<tuple_like Tuple>
concept set_of_types = is_unique_tuple<Tuple>;

} // namespace codys
