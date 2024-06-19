#pragma once

#include <tuple>

namespace codys {

// replace with std::tuple_like in C++23:
template <typename Tuple>
concept tuple_like = requires(Tuple tuple)
{
    std::get<0>(tuple);
    std::declval<std::tuple_element_t<0, std::remove_cvref_t<Tuple>>>();
    std::tuple_size_v<std::remove_cvref_t<Tuple>>;
};

} // namespace codys