#include "pips_data.hpp"

#include <fstream>
#include <print>

#include <nlohmann/json.hpp>

int main() {
    auto nyt_file = "data/pips.json";
    pips::download_file("https://www.nytimes.com/svc/pips/v1/2025-10-24.json", nyt_file);

    std::ifstream file(nyt_file);
    nlohmann::json data = nlohmann::json::parse(file);

    std::println("{}", data.dump());


    return 0;
}
