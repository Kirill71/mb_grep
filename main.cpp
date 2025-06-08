#include <iostream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

struct SearchOptions {
    std::string query;
    fs::path root_path;
    bool use_regex = false;
    bool ignore_case = false;
    std::optional<std::string> file_extension = std::nullopt;
};

class IMatcher {
public:
    virtual bool match(const std::string &line) const = 0;

    virtual ~IMatcher() = default;
};

class RegexMatcher final : public IMatcher {
    std::regex pattern_;

public:
    explicit RegexMatcher(const std::string &query, const bool ignore_case = false) {
        std::regex::flag_type flags = std::regex::ECMAScript;
        if (ignore_case) {
            flags |= std::regex::icase;
        }
        pattern_ = std::regex(query, flags);
    }

    bool match(const std::string &line) const override {
        return std::regex_search(line, pattern_);
    }
};

class SubstringMatcher final : public IMatcher {
    std::string query_;
    bool ignore_case_{};

public:
    explicit SubstringMatcher(std::string query, const bool ignore_case = false)
    : query_(std::move(query), ignore_case_(ignore_case)) {
        if (ignore_case_) {
            std::ranges::transform(query_, query_.begin(), tolower);
        }
    }
};


int main() {
}
