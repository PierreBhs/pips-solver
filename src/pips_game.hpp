#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace pips {

struct Domino
{
    std::uint8_t p1;
    std::uint8_t p2;
};

struct GridCell
{
    std::uint8_t row;
    std::uint8_t col;

    auto operator<=>(const GridCell&) const = default;
};

struct DominoPlacement
{
    Domino   domino;
    GridCell cell1;
    GridCell cell2;
};

enum class RegionType { EMPTY, EQUALS, SUM, LESS, GREATER, UNEQUAL };

struct Zone
{
    RegionType                  type;
    std::optional<std::uint8_t> target;
    std::vector<GridCell>       indices;
};

struct BoardDimensions
{
    std::uint8_t rows;
    std::uint8_t cols;
};

struct Game
{
    std::vector<Domino>                        dominoes;
    std::vector<Zone>                          zones;
    BoardDimensions                            dim;
    std::vector<std::pair<GridCell, GridCell>> official_solution;
};

}  // namespace pips
