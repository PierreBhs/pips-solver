#include "pips_data.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace pips
{

std::expected<NytJsonProvider, std::string> NytJsonProvider::create()
{
    NytJsonProvider provider;

    auto data_file_path = std::filesystem::current_path() / "data" / "pips.json";

    std::ifstream file(data_file_path);
    if (!file.is_open()) {
        return std::unexpected("Failed to open file: " + data_file_path.string());
    }

    provider.m_json_data = nlohmann::json::parse(file, nullptr, false);
    if (provider.m_json_data.is_discarded()) {
        return std::unexpected("Failed to parse JSON file: " + data_file_path.string());
    }

    if (auto result = provider.load_games_from_json(); !result) {
        return std::unexpected(result.error());
    }

    return provider;
}

const Game& NytJsonProvider::get_game(Difficulty difficulty) const
{
    return m_games[static_cast<size_t>(difficulty)];
}

std::expected<void, std::string> NytJsonProvider::load_games_from_json()
{
    static constexpr std::array<const char*, 3> difficulties = {"easy", "medium", "hard"};

    for (size_t i = 0; i < difficulties.size(); ++i) {
        if (!m_json_data.contains(difficulties[i])) {
            return std::unexpected("JSON data does not contain difficulty: " + std::string(difficulties[i]));
        }

        auto game_result = parse_game(m_json_data[difficulties[i]]);
        if (!game_result) {
            return std::unexpected(game_result.error());
        }

        m_games[i] = std::move(*game_result);
    }

    return {};
}

std::expected<Game, std::string> NytJsonProvider::parse_game(const nlohmann::json& game_json)
{
    if (!game_json.is_object()) {
        return std::unexpected("Game JSON is not an object.");
    }

    auto dominoes_result = parse_dominoes(game_json.value("dominoes", nlohmann::json()));
    if (!dominoes_result)
        return std::unexpected(dominoes_result.error());

    auto zones_result = parse_zones(game_json.value("regions", nlohmann::json()));
    if (!zones_result)
        return std::unexpected(zones_result.error());

    auto solution_result = parse_solution(game_json.value("solution", nlohmann::json()));
    if (!solution_result)
        return std::unexpected(solution_result.error());

    uint8_t max_row = 0, max_col = 0;
    for (const auto& zone : *zones_result) {
        for (const auto& cell : zone.indices) {
            max_row = std::max(max_row, cell.row);
            max_col = std::max(max_col, cell.col);
        }
    }

    return Game{.dominoes = std::move(*dominoes_result),
                .zones = std::move(*zones_result),
                .dim = {.rows = static_cast<uint8_t>(max_row + 1), .cols = static_cast<uint8_t>(max_col + 1)},
                .official_solution = std::move(*solution_result)};
}

std::expected<std::vector<Domino>, std::string> NytJsonProvider::parse_dominoes(const nlohmann::json& dominoes_json)
{
    if (!dominoes_json.is_array())
        return std::unexpected("Dominoes JSON is not an array.");

    std::vector<Domino> dominoes;
    dominoes.reserve(dominoes_json.size());
    for (const auto& domino_json : dominoes_json) {
        if (!domino_json.is_array() || domino_json.size() != 2)
            return std::unexpected("Invalid domino format.");
        dominoes.emplace_back(domino_json[0].get<uint8_t>(), domino_json[1].get<uint8_t>());
    }

    return dominoes;
}

std::expected<std::vector<Zone>, std::string> NytJsonProvider::parse_zones(const nlohmann::json& regions_json)
{
    if (!regions_json.is_array())
        return std::unexpected("Regions JSON is not an array.");

    std::vector<Zone> zones;
    zones.reserve(regions_json.size());

    for (const auto& region_json : regions_json) {
        if (!region_json.is_object())
            return std::unexpected("Region JSON is not an object.");

        auto type_str = region_json.value("type", "empty");

        auto target =
            region_json.contains("target") ? std::optional(region_json["target"].get<uint8_t>()) : std::nullopt;

        auto indices_json = region_json.value("indices", nlohmann::json());
        if (!indices_json.is_array())
            return std::unexpected("Indices JSON is not an array.");

        std::vector<GridCell> indices;
        indices.reserve(indices_json.size());

        for (const auto& index_json : indices_json) {
            if (!index_json.is_array() || index_json.size() != 2)
                return std::unexpected("Invalid index format.");
            indices.emplace_back(index_json[0].get<uint8_t>(), index_json[1].get<uint8_t>());
        }
        zones.emplace_back(to_region_type(type_str), target, std::move(indices));
    }

    return zones;
}

std::expected<NytJsonProvider::OfficialSolution, std::string> NytJsonProvider::parse_solution(
    const nlohmann::json& solution_json)
{
    if (!solution_json.is_array())
        return std::unexpected("Solution JSON is not an array.");

    OfficialSolution solution;
    solution.reserve(solution_json.size());

    for (const auto& placement_json : solution_json) {
        if (!placement_json.is_array() || placement_json.size() != 2)
            return std::unexpected("Invalid placement format.");

        const auto& cell1_json = placement_json[0];
        const auto& cell2_json = placement_json[1];
        if (!cell1_json.is_array() || cell1_json.size() != 2 || !cell2_json.is_array() || cell2_json.size() != 2) {
            return std::unexpected("Invalid cell format.");
        }

        solution.emplace_back(GridCell{cell1_json[0].get<uint8_t>(), cell1_json[1].get<uint8_t>()},
                              GridCell{cell2_json[0].get<uint8_t>(), cell2_json[1].get<uint8_t>()});
    }
    return solution;
}

RegionType NytJsonProvider::to_region_type(std::string_view region_str)
{
    if (region_str == "equals")
        return RegionType::EQUALS;
    if (region_str == "sum")
        return RegionType::SUM;
    if (region_str == "less")
        return RegionType::LESS;
    if (region_str == "greater")
        return RegionType::GREATER;
    if (region_str == "unequal")
        return RegionType::UNEQUAL;
    return RegionType::EMPTY;
}

}  // namespace pips
