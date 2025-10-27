#include "display.hpp"
#include "pips_data.hpp"
#include "solver.hpp"

#include <chrono>
#include <iostream>
#include <print>

[[nodiscard]] std::expected<void, std::string> fetch_daily_pips()
{
    // Get current date in YYYY-MM-DD format
    const auto now = std::chrono::system_clock::now();
    const auto tt = std::chrono::system_clock::to_time_t(now);
    const auto tm = *std::localtime(&tt);

    const std::string date_str = std::format("{:04}-{:02}-{:02}", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    const std::string           url = "https://www.nytimes.com/svc/pips/v1/" + date_str + ".json";
    const std::filesystem::path output_path = std::filesystem::current_path() / "data" / "pips.json";

    std::filesystem::create_directories(output_path.parent_path());

    const std::string cmd = std::format("curl -s -f -o \"{}\" \"{}\"", output_path.string(), url);

    if (const int result = std::system(cmd.c_str()); result != 0) {
        return std::unexpected(
            std::format("Failed to download puzzle for {} (curl exited with code {}). "
                        "Check that curl is installed.",
                        date_str,
                        result));
    }

    return {};
}

int main()
{
    if (auto fetch_result = fetch_daily_pips(); !fetch_result) {
        std::println(std::cerr, "Error: {}", fetch_result.error());
        return 1;
    }

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

        pips::Solver                        solver(game);
        const auto                          start_time = std::chrono::high_resolution_clock::now();
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
