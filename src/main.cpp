#include "pips_data.hpp"
#include "solver.hpp"

#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <vector>

// Display related
void        print_game(const pips::Game& game, pips::NytJsonProvider::Difficulty difficulty);
std::string format_domino_list(const std::vector<pips::Domino>& dominoes);
std::string format_gridcell_list(const std::vector<pips::GridCell>& cells);
std::string format_official_solution(const std::vector<std::pair<pips::GridCell, pips::GridCell>>& solution);
std::string format_solver_solution(const std::vector<pips::DominoPlacement>& solution);
std::string to_string(pips::RegionType type);

bool verify_solution(const std::optional<std::vector<pips::DominoPlacement>>&      solver_solution,
                     const std::vector<std::pair<pips::GridCell, pips::GridCell>>& official_solution);

int main()
{
    auto provider_or_error = pips::NytJsonProvider::create("data/pips.json");
    if (!provider_or_error) {
        std::println(std::cerr, "Error: {}", provider_or_error.error());
        return 1;
    }

    const auto& provider = *provider_or_error;

    std::println("\n┌──────────────────────────────────────────┐");
    std::println("│       Pips Daily Game Data from NYT      │");
    std::println("└──────────────────────────────────────────┘");

    const auto& hard_game = provider.get_game(pips::NytJsonProvider::Difficulty::HARD);
    print_game(hard_game, pips::NytJsonProvider::Difficulty::HARD);

    std::println("\n◆ Running solver...");
    pips::Solver solver(hard_game);
    auto         solution_opt = solver.solve();

    if (solution_opt) {
        std::println("  Solver found a solution!");
        std::println("  {}", format_solver_solution(*solution_opt));
    } else {
        std::println("  Solver could not find a solution.");
    }

    std::println("\n◆ Verifying against official solution...");
    if (verify_solution(*solution_opt, hard_game .official_solution)) {
        std::println("  ✅ Verification PASSED");
    } else {
        std::println("  ❌ Verification FAILED");
    }

    return 0;
}

void print_game(const pips::Game& game, pips::NytJsonProvider::Difficulty difficulty)
{
    auto difficulty_to_string = [](pips::NytJsonProvider::Difficulty d) {
        switch (d) {
            case pips::NytJsonProvider::Difficulty::EASY:
                return "Easy";
            case pips::NytJsonProvider::Difficulty::MEDIUM:
                return "Medium";
            case pips::NytJsonProvider::Difficulty::HARD:
                return "Hard";
        }
        return "Unknown";
    };

    std::println("\n╔═══════════════════════════════════════════╗");
    std::println("║   GAME: {:^31}   ║", difficulty_to_string(difficulty));
    std::println("╚═══════════════════════════════════════════╝");

    std::println("\n◆ Dimensions: {} rows x {} cols", game.dim.rows, game.dim.cols);
    std::println("\n◆ Dominoes Set (Count: {})", game.dominoes.size());
    std::println("  {}", format_domino_list(game.dominoes));

    std::println("\n◆ Zone Rules:");
    if (game.zones.empty()) {
        std::println("  No zones defined.");
    } else {
        const size_t type_col_width = std::ranges::max(
            game.zones | std::views::transform([](const auto& z) { return to_string(z.type).length(); }));

        std::println("  {:<{}} │ {:^8} │ {}", "Type", type_col_width, "Target", "Indices");
        std::println("  {:─^{}}─┼{:─^9}─┼{:─<30}", "", type_col_width, "", "");

        for (const auto& zone : game.zones) {
            std::string type_str{to_string(zone.type)};

            std::string target_str = zone.target ? std::format("{:^8}", *zone.target) : std::format("{:^8}", "---");

            std::println(
                "  {:<{}} │ {} │ {}", type_str, type_col_width, target_str, format_gridcell_list(zone.indices));
        }
    }
    std::println("\n◆ Official Solution ({} placements):", game.official_solution.size());
    std::println("  {}", format_official_solution(game.official_solution));
}

std::string format_domino_list(const std::vector<pips::Domino>& dominoes)
{
    if (dominoes.empty())
        return "";

    std::string result;
    for (const auto& d : dominoes) {
        result += std::format("({},{}), ", d.p1, d.p2);
    }
    result.pop_back();
    result.pop_back();

    return result;
}

std::string format_gridcell_list(const std::vector<pips::GridCell>& cells)
{
    if (cells.empty())
        return "";

    std::string result;
    for (const auto& c : cells) {
        result += std::format("({},{}), ", c.row, c.col);
    }
    result.pop_back();
    result.pop_back();

    return result;
}

std::string format_official_solution(const std::vector<std::pair<pips::GridCell, pips::GridCell>>& solution)
{
    if (solution.empty())
        return "";

    std::string result;
    for (const auto& p : solution) {
        result += std::format("[({},{})-({},{})], ", p.first.row, p.first.col, p.second.row, p.second.col);
    }
    result.pop_back();
    result.pop_back();

    return result;
}

std::string format_solver_solution(const std::vector<pips::DominoPlacement>& solution)
{
    if (solution.empty())
        return "";

    std::string result;
    for (const auto& p : solution) {
        result += std::format("[D:({},{}) as ({} @ {},{})-({} @ {},{})], ",
                              p.original_domino.p1,
                              p.original_domino.p2,
                              p.placement1.pip,
                              p.placement1.cell.row,
                              p.placement1.cell.col,
                              p.placement2.pip,
                              p.placement2.cell.row,
                              p.placement2.cell.col);
    }
    result.pop_back();
    result.pop_back();  // Remove trailing ", "

    return result;
}

std::string to_string(pips::RegionType type)
{
    switch (type) {
        case pips::RegionType::EMPTY:
            return "Empty";
        case pips::RegionType::EQUALS:
            return "Equals";
        case pips::RegionType::SUM:
            return "Sum";
        case pips::RegionType::LESS:
            return "Less";
        case pips::RegionType::GREATER:
            return "Greater";
        case pips::RegionType::UNEQUAL:
            return "Unequal";
    }
    return "Unknown";
}

std::pair<pips::GridCell, pips::GridCell> normalize_placement(pips::GridCell c1, pips::GridCell c2)
{
    if (c1 < c2) {
        return {c1, c2};
    }
    return {c2, c1};
}

bool verify_solution(const std::optional<std::vector<pips::DominoPlacement>>&      solver_solution_opt,
                     const std::vector<std::pair<pips::GridCell, pips::GridCell>>& official_solution)
{
    if (!solver_solution_opt) {
        std::println(std::cerr, "  Verification skipped: Solver did not produce a solution.");
        return false;
    }

    const auto& solver_placements = *solver_solution_opt;
    if (solver_placements.size() != official_solution.size()) {
        std::println(std::cerr,
                     "  Verification failed: Different number of placements (Solver: {}, Official: {}).",
                     solver_placements.size(),
                     official_solution.size());
        return false;
    }

    std::vector<std::pair<pips::GridCell, pips::GridCell>> normalized_solver_solution;
    normalized_solver_solution.reserve(solver_placements.size());
    for (const auto& placement : solver_placements) {
        // Access the cells from the PlacedPip structs
        normalized_solver_solution.push_back(normalize_placement(placement.placement1.cell, placement.placement2.cell));
    }

    std::ranges::sort(normalized_solver_solution);

    std::vector<std::pair<pips::GridCell, pips::GridCell>> normalized_official_solution;
    normalized_official_solution.reserve(official_solution.size());

    for (const auto& placement : official_solution) {
        normalized_official_solution.push_back(normalize_placement(placement.first, placement.second));
    }

    std::ranges::sort(normalized_official_solution);

    return normalized_solver_solution == normalized_official_solution;
}
