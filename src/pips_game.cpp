#include "pips_game.hpp"

#include <cmath>

namespace pips {

bool GridCell::is_adjacent(const GridCell& other) const noexcept
{
    const auto dx = static_cast<int8_t>(col - other.col);
    const auto dy = static_cast<int8_t>(row - other.row);
    return (dx == 0 && std::abs(dy) == 1) || (dy == 0 && std::abs(dx) == 1);
}

}  // namespace pips
