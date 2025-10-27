#pragma once

#include <optional>
#include <vector>
#include "pips_game.hpp"

namespace pips {

class Solver
{
public:
    explicit Solver(const Game& game);

    [[nodiscard]] std::optional<std::vector<DominoPlacement>> solve();

private:
    bool backtrack();

    std::optional<GridCell> find_unoccupied_cell() const;

    bool check_zone_constraints(const Zone& zone) const;

    const Game&                      m_game;
    std::vector<std::vector<int8_t>> m_grid;
    std::vector<bool>                m_used_dominoes;
    // used to print the solution, not needed to solve
    std::vector<DominoPlacement>          m_solution_placements;
    std::vector<std::vector<const Zone*>> m_zone_lookup;

    // Values in the grid, -2 since puzzles can have void spots
    // -1 when no domino is placed, otherwise the value of the domino
    static constexpr std::int8_t UNOCCUPIED = -1;
    static constexpr std::int8_t HOLE = -2;
};

}  // namespace pips
