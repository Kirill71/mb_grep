#include "utils.h"

#include <fstream>
#include <regex>
#include <thread>

namespace mb {
bool is_binary_file(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    char buffer[512];
    file.read(buffer, sizeof(buffer));
    const auto bytesRead = file.gcount();
    return std::any_of(buffer, buffer + bytesRead, [](const char c) { return c == '\0'; });
}

bool contains_regex_chars(const std::string& query) {
    static const std::regex likely_regex_pattern(R"([.^$*+?{}\[\]\\|()])");
    return std::regex_search(query, likely_regex_pattern);
}

size_t get_threads_number() {
    constexpr size_t reservedThreads = 2;
    auto num_threads = static_cast<size_t>(std::thread::hardware_concurrency());
    num_threads = std::max<size_t>(1, num_threads > reservedThreads ? num_threads - reservedThreads : 1);
    return num_threads;
}

} // namespace mb