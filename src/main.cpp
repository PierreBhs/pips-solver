#include "display.hpp"
#include "pips_data.hpp"
#include "solver.hpp"

#include <chrono>
#include <iostream>
#include <print>

int main()
{
    auto provider_or_error = pips::NytJsonProvider::create();
    if (!provider_or_error) {
        std::println(std::cerr, "Error: {}", provider_or_error.error());
        return 1;
    }

    const auto& provider = *provider_or_error;

    for (auto difficulty : {pips::NytJsonProvider::Difficulty::EASY,
                            pips::NytJsonProvider::Difficulty::MEDIUM,
                            pips::NytJsonProvider::Difficulty::HARD}) {
        const auto& game = provider.get_game(difficulty);

        const auto                          start_time = std::chrono::high_resolution_clock::now();
        pips::Solver                        solver(game);
        auto                                solution_opt = solver.solve();
        const auto                          end_time = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double> solver_time = end_time - start_time;

        if (solution_opt) {
            pips::print_game_solution(game, *solution_opt, solver_time, difficulty);
        } else {
            std::println("Solver could not find a solution.");
        }
    }

    return 0;
}
