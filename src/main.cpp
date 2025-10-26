#include "pips_data.hpp"

#include <algorithm>
#include <format>
#include <iostream>  // cerr
#include <print>
#include <ranges>
#include <string>

void        print_game(const pips::Game& game, pips::NytJsonProvider::Difficulty difficulty);
std::string format_pair_list(const auto& pairs);
std::string format_solution_list(const pips::Solution& solution);
std::string to_string(pips::RegionType type);

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

    const auto& easy_game = provider.get_game(pips::NytJsonProvider::Difficulty::EASY);
    print_game(easy_game, pips::NytJsonProvider::Difficulty::EASY);

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

    std::println("\n◆ Dominoes Set (Count: {})", game.dominoes.size());
    std::println("  {}", format_pair_list(game.dominoes));

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

            std::println("  {:<{}} │ {} │ {}", type_str, type_col_width, target_str, format_pair_list(zone.indices));
        }
    }

    std::println("\n◆ Solution ({} placements):", game.solution.size());
    std::println("  {}", format_solution_list(game.solution));
}

std::string format_pair_list(const auto& pairs)
{
    if (pairs.empty()) {
        return "";
    }

    std::string result;

    const auto& first_pair = pairs.front();
    result += std::format("({},{})", first_pair.first, first_pair.second);

    for (size_t i = 1; i < pairs.size(); ++i) {
        const auto& p = pairs[i];
        result += std::format(", ({},{})", p.first, p.second);
    }

    return result;
}

std::string format_solution_list(const pips::Solution& solution)
{
    if (solution.empty()) {
        return "";
    }

    std::string result;

    const auto& first_placement = solution.front();
    result += std::format("(( {} ,{} )-( {} ,{} ))",
                          first_placement.first.first,
                          first_placement.first.second,
                          first_placement.second.first,
                          first_placement.second.second);

    for (size_t i = 1; i < solution.size(); ++i) {
        const auto& p = solution[i];
        result += std::format(", (({},{})-({},{}))", p.first.first, p.first.second, p.second.first, p.second.second);
    }

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
