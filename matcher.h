#pragma once
#include <regex>
#include <string>

namespace mb {
class IMatcher {
public:
    virtual bool match(const std::string& line) const = 0;

    virtual ~IMatcher() = default;
};

class RegexMatcher final : public IMatcher {
public:
    explicit RegexMatcher(const std::string& query, bool ignore_case = false);

    bool match(const std::string& line) const override;

private:
    std::regex pattern_;
};

class SubstringMatcher final : public IMatcher {
public:
    explicit SubstringMatcher(std::string query, bool ignore_case = false);

    bool match(const std::string& line) const override;

private:
    std::string query_;
    bool ignore_case_{};
};
} // namespace mb