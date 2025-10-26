#pragma once

#include "pips_data.hpp"

#include <chrono>
#include <vector>

namespace pips
{
    void print_game_solution(const pips::Game& game,
                             const std::vector<pips::DominoPlacement>& solution,
                             const std::chrono::duration<double>& solver_time,
                             pips::NytJsonProvider::Difficulty difficulty);
}
