#include "solver.hpp"

#include <algorithm>
#include <numeric>
#include <ranges>

namespace pips {

Solver::Solver(const Game& game) : m_game(game), m_used_dominoes(game.dominoes.size(), false)
{

    const auto& [rows, cols] = game.dim;
    m_grid.assign(rows, std::vector<int8_t>(cols, HOLE));
    m_zone_lookup.assign(rows, std::vector<const Zone*>(cols, nullptr));

    for (const auto& zone : game.zones) {
        for (const auto& cell : zone.indices) {
            m_grid[cell.row][cell.col] = UNOCCUPIED;
            m_zone_lookup[cell.row][cell.col] = &zone;
        }
    }
}

std::optional<std::vector<DominoPlacement>> Solver::solve()
{
    if (backtrack()) {
        return m_solution_placements;
    }

    return std::nullopt;
}

std::optional<GridCell> Solver::find_unoccupied_cell() const
{
    for (std::uint8_t r = 0; r < m_game.dim.rows; ++r) {
        for (std::uint8_t c = 0; c < m_game.dim.cols; ++c) {
            if (m_grid[r][c] == UNOCCUPIED) {
                return GridCell{r, c};
            }
        }
    }

    return std::nullopt;
}

bool Solver::backtrack()
{
    const auto next_cell_opt = find_unoccupied_cell();
    if (!next_cell_opt) {
        return true;
    }
    const auto& cell = *next_cell_opt;

    for (std::size_t i = 0; i < m_game.dominoes.size(); ++i) {
        if (m_used_dominoes[i])
            continue;

        const auto& domino = m_game.dominoes[i];

        auto try_placement = [&](const GridCell& c1, const GridCell& c2, std::uint8_t p1, std::uint8_t p2) -> bool {
            if (c2.row >= m_game.dim.rows || c2.col >= m_game.dim.cols || m_grid[c2.row][c2.col] != UNOCCUPIED) {
                return false;
            }

            m_grid[c1.row][c1.col] = p1;
            m_grid[c2.row][c2.col] = p2;
            m_used_dominoes[i] = true;

            const Zone* zone1 = m_zone_lookup[c1.row][c1.col];
            const Zone* zone2 = m_zone_lookup[c2.row][c2.col];

            bool is_valid = check_zone_constraints(*zone1);
            if (is_valid && zone1 != zone2) {
                is_valid = check_zone_constraints(*zone2);
            }

            if (is_valid) {
                // captures which pip went to which cell.
                m_solution_placements.push_back({domino, {c1, p1}, {c2, p2}});
                if (backtrack())
                    return true;
                m_solution_placements.pop_back();  // Backtrack
            }

            m_grid[c1.row][c1.col] = UNOCCUPIED;
            m_grid[c2.row][c2.col] = UNOCCUPIED;
            m_used_dominoes[i] = false;

            return false;
        };

        const GridCell horiz_cell = {cell.row, static_cast<std::uint8_t>(cell.col + 1)};

        if (try_placement(cell, horiz_cell, domino.p1, domino.p2))
            return true;

        if (domino.p1 != domino.p2) {
            if (try_placement(cell, horiz_cell, domino.p2, domino.p1))
                return true;
        }

        const GridCell vert_cell = {static_cast<std::uint8_t>(cell.row + 1), cell.col};

        if (try_placement(cell, vert_cell, domino.p1, domino.p2))
            return true;

        if (domino.p1 != domino.p2) {
            if (try_placement(cell, vert_cell, domino.p2, domino.p1))
                return true;
        }
    }

    return false;
}

bool Solver::check_zone_constraints(const Zone& zone) const
{
    std::vector<std::uint8_t> pips_in_zone;
    bool                is_zone_full = true;

    for (const auto& cell : zone.indices) {
        const auto pip_val = m_grid[cell.row][cell.col];
        if (pip_val != UNOCCUPIED) {
            pips_in_zone.push_back(pip_val);
        } else {
            is_zone_full = false;
        }
    }

    if (pips_in_zone.empty())
        return true;

    switch (zone.type) {
        case RegionType::SUM: {
            auto current_sum = std::accumulate(pips_in_zone.begin(), pips_in_zone.end(), 0);
            if (current_sum > zone.target.value())
                return false;
            if (is_zone_full && current_sum != zone.target.value())
                return false;
            break;
        }
        case RegionType::GREATER:
            if (is_zone_full && std::accumulate(pips_in_zone.begin(), pips_in_zone.end(), 0) <= zone.target.value())
                return false;
            break;
        case RegionType::LESS:
            if (is_zone_full && std::accumulate(pips_in_zone.begin(), pips_in_zone.end(), 0) >= zone.target.value())
                return false;
            break;
        case RegionType::EQUALS:
            for (auto pip : pips_in_zone | std::views::drop(1)) {
                if (pip != pips_in_zone.front())
                    return false;
            }
            break;
        case RegionType::UNEQUAL:
            if (pips_in_zone.size() > 1) {
                auto sorted_pips = pips_in_zone;
                std::ranges::sort(sorted_pips);
                for (std::size_t i = 0; i < sorted_pips.size() - 1; ++i) {
                    if (sorted_pips[i] == sorted_pips[i + 1])
                        return false;
                }
            }
            break;
        case RegionType::EMPTY:
            break;
    }
    return true;
}
}  // namespace pips
