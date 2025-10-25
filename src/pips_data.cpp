#include "pips_data.hpp"

#include <cpr/cpr.h>

#include <fstream>
#include <print>

#include <algorithm>  // For std::ranges::max
#include <format>     // For std::format
#include <ranges>     // For views
#include <string>     // For std::string

namespace {
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
    result += std::format("(({},{})-({},{}))",
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

}  // namespace

namespace pips {

void download_file(std::string_view url, std::string_view output_path)
{
    cpr::Response r = cpr::Get(cpr::Url{url});

    if (r.status_code == 200) {
        std::ofstream output_file(output_path.data(), std::ios::binary);
        if (output_file) {
            output_file.write(r.text.c_str(), r.text.length());
            std::println("File downloaded successfully to {}", output_path);
        } else {
            std::println(stderr, "Error: Could not open file for writing at ", output_path);
        }
    } else {
        std::println(stderr, "Error downloading file: {}", r.status_code);
    }
}

RegionType to_region_type(std::string_view region)
{
    static constexpr std::array<std::pair<std::string_view, RegionType>, 6> string_to_type = {
        {{"empty", RegionType::EMPTY},
         {"equals", RegionType::EQUALS},
         {"sum", RegionType::SUM},
         {"less", RegionType::LESS},
         {"greater", RegionType::GREATER},
         {"unequal", RegionType::UNEQUAL}}};

    if (auto it = std::ranges::find(string_to_type, region, &std::pair<std::string_view, RegionType>::first);
        it != string_to_type.end()) {
        return it->second;
    }

    return RegionType::EMPTY;
}

constexpr std::string_view to_string(RegionType type) noexcept
{
    switch (type) {
        case RegionType::EMPTY:
            return "empty";
        case RegionType::EQUALS:
            return "equals";
        case RegionType::SUM:
            return "sum";
        case RegionType::LESS:
            return "less";
        case RegionType::GREATER:
            return "greater";
        case RegionType::UNEQUAL:
            return "unequal";
    }

    std::unreachable();
}

NytGameHandler::NytGameHandler(std::string_view file_path) : m_json_data(), m_games()
{

    std::ifstream file{file_path.data()};
    m_json_data = nlohmann::json::parse(file);
    from_json();
}

const Dominoes& NytGameHandler::get_dominoes(Difficulty difficulty) const
{
    return m_games[difficulty_to_index(difficulty)].dominoes;
}

void NytGameHandler::print() const
{
    std::println("\n┌──────────────────────────────────────────┐");
    std::println("│       Pips Daily Game Data from NYT      │");
    std::println("└──────────────────────────────────────────┘");

    for (std::size_t i = 0; i < m_games.size(); ++i) {
        const auto& game = m_games[i];
        const auto  difficulty = static_cast<Difficulty>(i);

        std::println("\n╔═══════════════════════════════════════════╗");
        std::println("║   GAME: {:^31}   ║", difficulty_to_string(difficulty));
        std::println("╚═══════════════════════════════════════════╝");

        std::println("\n◆ Dominoes Set (Count: {}):", game.dominoes.size());
        std::println("  {}", format_pair_list(game.dominoes));

        std::println("\n◆ Zone Rules:");
        if (game.zones.empty()) {
            std::println("  No zones defined.");
            continue;
        }

        const size_t type_col_width = std::ranges::max(
            game.zones | std::views::transform([](const auto& z) { return to_string(z.type).length(); }));

        std::println("  {:<{}} │ {:^8} │ {}", "Type", type_col_width, "Target", "Indices");
        std::println("  {:─^{}}─┼{:─^9}─┼{:─<30}", "", type_col_width, "", "");

        for (const auto& zone : game.zones) {
            std::string type_str{to_string(zone.type)};

            std::string target_str = zone.target ? std::format("{:^8}", *zone.target) : std::format("{:^8}", "---");

            std::println("  {:<{}} │ {} │ {}", type_str, type_col_width, target_str, format_pair_list(zone.indices));
        }

        std::println("\n◆ Solution ({} placements):", game.solution.size());
        std::println("  {}", format_solution_list(game.solution));
    }
}

void NytGameHandler::from_json()
{
    static constexpr std::array<std::string, 3> difficulties = {"easy", "medium", "hard"};

    for (auto i = 0ul; i < difficulties.size(); ++i) {
        const auto& game_json = m_json_data[difficulties[i]];
        auto&       game_data = m_games[i];

        game_data.dominoes = json_to_vector_of_pairs<Domino>(game_json["dominoes"]);

        for (const auto& region_json : game_json["regions"]) {
            auto type = to_region_type(region_json["type"].template get<std::string>());

            std::optional<std::uint8_t> target;
            if (region_json.contains("target")) {
                target = region_json["target"].get<std::uint8_t>();
            }

            auto indices = json_to_vector_of_pairs<std::pair<std::uint8_t, std::uint8_t>>(region_json["indices"]);

            game_data.zones.emplace_back(type, target, indices);
        }

        game_data.solution = parse_solution(game_json["solution"]);
    }
}

constexpr std::size_t NytGameHandler::difficulty_to_index(Difficulty difficulty) const
{
    switch (difficulty) {
        case Difficulty::EASY:
            return 0;
        case Difficulty::MEDIUM:
            return 1;
        case Difficulty::HARD:
            return 2;
    }
    std::unreachable();
}

constexpr std::string_view NytGameHandler::difficulty_to_string(Difficulty difficulty) const
{
    switch (difficulty) {
        case NytGameHandler::Difficulty::EASY:
            return "Easy";
        case NytGameHandler::Difficulty::MEDIUM:
            return "Medium";
        case NytGameHandler::Difficulty::HARD:
            return "Hard";
    }

    std::unreachable();
}

template <typename PairType>
std::vector<PairType> NytGameHandler::json_to_vector_of_pairs(const nlohmann::json& j)
{
    if (!j.is_array())
        return {};

    // clang-format off
    auto to_pair_view =
        j | std::views::transform([](const auto& arr) {
                using FirstType = typename PairType::first_type;
                using SecondType = typename PairType::second_type;
                return PairType{arr[0].template get<FirstType>(), arr[1].template get<SecondType>()};
    });
    // clang-format on

    return {to_pair_view.begin(), to_pair_view.end()};
}

Solution NytGameHandler::parse_solution(const nlohmann::json& j)
{
    if (!j.is_array()) {
        return {};
    }

    auto to_domino_placement = [](const nlohmann::json& placement_json) -> DominoPlacement {
        const auto& cell1_json = placement_json[0];
        const auto& cell2_json = placement_json[1];

        return {{cell1_json[0].get<std::uint8_t>(), cell1_json[1].get<std::uint8_t>()},
                {cell2_json[0].get<std::uint8_t>(), cell2_json[1].get<std::uint8_t>()}};
    };

    return j | std::views::transform(to_domino_placement) | std::ranges::to<Solution>();
}

}  // namespace pips
