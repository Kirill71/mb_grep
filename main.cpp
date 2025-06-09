#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>

#include "matcher.h"
#include "thread_pool.h"
#include "utils.h"

namespace fs = std::filesystem;
namespace mb {
struct SearchOptions {
    std::string query{};
    fs::path root_path{};
    bool use_regex = false;
    bool ignore_case = false;
    std::optional<std::string> file_extension = std::nullopt;
};

void search_file(const fs::path& filePath, const IMatcher& matcher, std::mutex& output_mutex) {
    std::ifstream file{filePath};
    if (!file) {
        return;
    }
    std::string line;
    size_t line_num = 0;
    while (std::getline(file, line)) {
        ++line_num;
        if (matcher.match(line)) {
            std::lock_guard lock{output_mutex};
            std::cout << filePath << ":" << line_num << ": " << line << std::endl;
        }
    }
}

void walk_directory(const SearchOptions& options, ThreadPool& pool, const IMatcher& matcher, std::mutex& output_mtx) {
    const auto& root_path = options.root_path;
    const auto& file_extension = options.file_extension;
    for (auto& entry : fs::recursive_directory_iterator(root_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto& path = entry.path();
        if (is_binary_file(path)) {
            continue;
        }
        if (file_extension.has_value() && file_extension.value() != path.extension()) {
            continue;
        }
        pool.submit([path, &matcher, &output_mtx] { search_file(path, matcher, output_mtx); });
    }
}

std::unique_ptr<IMatcher> create_matcher(const SearchOptions& options) {
    if (options.use_regex) {
        return std::make_unique<RegexMatcher>(options.query, options.ignore_case);
    }
    return std::make_unique<SubstringMatcher>(options.query, options.ignore_case);
}

SearchOptions extract_arguments(const int argc, char* argv[]) {
    SearchOptions options{};
    options.query = argv[1];
    options.root_path = argv[2];
    for (int i = 3; i < argc; ++i) {
        if (std::string arg = argv[i]; arg == "--regex") {
            options.use_regex = true;
        } else if (arg == "--ignore-case") {
            options.ignore_case = true;
        } else if (arg.rfind("--ext=", 0) == 0) {
            options.file_extension = arg.substr(6);
        }
    }
    return options;
}
} // namespace mb

static void help(const std::string& program_name) {
    std::cerr << "Usage: " << program_name << " <query> <directory> [--regex] [--ignore-case] [--ext=.txt]"
              << std::endl;
}

int main(int argc, char* argv[]) {
    constexpr int minimalArgCount = 3;
    if (argc < minimalArgCount) {
        help(argv[0]);
        return 1;
    }
    try {
        auto options = mb::extract_arguments(argc, argv);
        if (!options.use_regex && mb::contains_regex_chars(options.query)) {
            std::cerr << "Warning: The pattern \"" << options.query
                      << "\" looks like a regular expression, but --regex flag was not set.\n";
            return 1;
        }
        auto matcher = mb::create_matcher(options);
        auto num_threads = mb::get_threads_number();
        mb::ThreadPool pool{num_threads};
        std::mutex output_mutex{};
        walk_directory(options, pool, *matcher, output_mutex);
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception!" << std::endl;
    }
    return 0;
}
