#include "pips_data.hpp"

#include <cpr/cpr.h>

#include <fstream>
#include <print>

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
}  // namespace pips
