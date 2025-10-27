#include "solver.hpp"

#include <algorithm>
#include <numeric>

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

    // generate all valid placements for a domino at the current cell
    const auto enumerate_placements = [&](const Domino& domino) {
        std::vector<std::tuple<GridCell, GridCell, uint8_t, uint8_t>> placements;

        const auto try_add = [&](GridCell c2, uint8_t p1, uint8_t p2) {
            if (c2.row < m_game.dim.rows && c2.col < m_game.dim.cols && m_grid[c2.row][c2.col] == UNOCCUPIED) {
                placements.emplace_back(cell, c2, p1, p2);
            }
        };

        // Horizontal
        try_add({cell.row, static_cast<std::uint8_t>(cell.col + 1)}, domino.p1, domino.p2);
        if (domino.p1 != domino.p2) {
            try_add({cell.row, static_cast<std::uint8_t>(cell.col + 1)}, domino.p2, domino.p1);
        }

        // Vertical
        try_add({static_cast<std::uint8_t>(cell.row + 1), cell.col}, domino.p1, domino.p2);
        if (domino.p1 != domino.p2) {
            try_add({static_cast<std::uint8_t>(cell.row + 1), cell.col}, domino.p2, domino.p1);
        }

        return placements;
    };

    for (std::size_t i = 0; i < m_game.dominoes.size(); ++i) {
        if (m_used_dominoes[i])
            continue;

        const auto& domino = m_game.dominoes[i];
        for (const auto& [c1, c2, p1, p2] : enumerate_placements(domino)) {
            // Apply placement
            m_grid[c1.row][c1.col] = p1;
            m_grid[c2.row][c2.col] = p2;
            m_used_dominoes[i] = true;

            auto* zone1 = m_zone_lookup[c1.row][c1.col];
            auto* zone2 = m_zone_lookup[c2.row][c2.col];

            // Validate zones
            bool valid = check_zone_constraints(*zone1);
            if (valid && zone1 != zone2) {
                valid = check_zone_constraints(*zone2);
            }

            if (valid) {
                m_solution_placements.push_back({domino, {c1, p1}, {c2, p2}});
                if (backtrack())
                    return true;
                // undo path
                m_solution_placements.pop_back();
            }

            // Undo
            m_grid[c1.row][c1.col] = UNOCCUPIED;
            m_grid[c2.row][c2.col] = UNOCCUPIED;
            m_used_dominoes[i] = false;
        }
    }

    return false;
}

bool Solver::check_zone_constraints(const Zone& zone) const
{
    std::vector<std::uint8_t> pips_in_zone;
    bool                      is_zone_full = true;

    for (const auto& [row, col] : zone.indices) {
        const auto pip_val = m_grid[row][col];
        if (pip_val != UNOCCUPIED) {
            pips_in_zone.push_back(pip_val);
        } else {
            is_zone_full = false;
        }
    }

    if (pips_in_zone.empty()) {
        return true;
    }

    switch (zone.type) {
        case RegionType::SUM: {
            auto current_sum = std::accumulate(pips_in_zone.begin(), pips_in_zone.end(), 0);
            if (current_sum > zone.target.value()) {
                return false;
            }
            if (is_zone_full && current_sum != zone.target.value()) {
                return false;
            }
            break;
        }
        case RegionType::GREATER:
            if (is_zone_full && std::accumulate(pips_in_zone.begin(), pips_in_zone.end(), 0) <= zone.target.value()) {
                return false;
            }
            break;
        case RegionType::LESS:
            if (is_zone_full && std::accumulate(pips_in_zone.begin(), pips_in_zone.end(), 0) >= zone.target.value()) {
                return false;
            }
            break;
        case RegionType::EQUALS:
            if (std::ranges::adjacent_find(pips_in_zone, std::ranges::not_equal_to()) != pips_in_zone.end()) {
                return false;
            }

            break;
        case RegionType::UNEQUAL:
            if (pips_in_zone.size() > 1) {
                auto sorted_pips = pips_in_zone;
                std::ranges::sort(sorted_pips);
                if (std::ranges::adjacent_find(sorted_pips) != sorted_pips.end()) {
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
