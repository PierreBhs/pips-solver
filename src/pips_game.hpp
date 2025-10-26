#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace pips {

using Domino = std::pair<std::uint8_t, std::uint8_t>;
using Dominoes = std::vector<Domino>;

enum class RegionType { EMPTY, EQUALS, SUM, LESS, GREATER, UNEQUAL };

struct Zone
{
    RegionType                                         type;
    std::optional<std::uint8_t>                        target;
    std::vector<std::pair<std::uint8_t, std::uint8_t>> indices;
};

using Zones = std::vector<Zone>;
using BoardDim = std::pair<std::uint8_t, std::uint8_t>;

// The solution is an array of domino placements,
// where each placement is a pair of coordinate pairs
using GridCell = std::pair<std::uint8_t, std::uint8_t>;
using DominoPlacement = std::pair<GridCell, GridCell>;
using Solution = std::vector<DominoPlacement>;

struct Game
{
    Dominoes dominoes;
    Zones    zones;
    Solution solution;
    BoardDim dim;
};

}  // namespace pips
