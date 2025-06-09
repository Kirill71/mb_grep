#include <iostream>
#include <filesystem>
#include <regex>
#include <fstream>
#include <thread>
#include <atomic>
#include <optional>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace fs = std::filesystem;

struct SearchOptions {
    std::string query {};
    fs::path root_path {};
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
        : query_(std::move(query)), ignore_case_(ignore_case) {
        if (ignore_case_) {
            std::ranges::transform(query_, query_.begin(), tolower);
        }
    }

    bool match(const std::string &line) const override {
        if (ignore_case_) {
            std::string lower_line = line;
            std::ranges::transform(lower_line, lower_line.begin(), tolower);
            return lower_line.find(query_) != std::string::npos;
        }
        return line.find(query_) != std::string::npos;
    }
};

bool is_binary_file(const fs::path &path) {
    std::ifstream file(path, std::ios::binary);
    char buffer[512];
    file.read(buffer, sizeof(buffer));
    const auto bytesRead = file.gcount();
    return std::any_of(buffer, buffer + bytesRead, [](const char c) {
        return c == '\0';
    });
}

class ThreadPool {
    using Task = std::function<void()>;
    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic_bool stop_flag_ = false;

public:
    explicit ThreadPool(const size_t num_threads) {
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    Task task {};
                    {
                        std::unique_lock lock {queue_mutex_};
                        condition_.wait(lock, [this]{ return stop_flag_ || !tasks_.empty(); });
                        if (stop_flag_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        stop_flag_ = true;
        condition_.notify_all();
        for (auto& worker: workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void submit(Task task) {
        {
            std::lock_guard lock {queue_mutex_};
            tasks_.push(std::move(task));
        }
        condition_.notify_one();
    }
};

void search_file(const fs::path& filePath, const IMatcher &matcher, std::mutex &output_mutex) {
   std::ifstream file {filePath};
    if (!file) {
        return;
    }
    std::string line;
    size_t line_num = 0;
    while (std::getline(file, line)) {
        ++line_num;
        if (matcher.match(line)) {
            std::lock_guard lock {output_mutex};
            std::cout << filePath << ":" << line_num << ": " << line << std::endl;
        }
    }
}

void walk_directory(const SearchOptions &options, ThreadPool &pool, const IMatcher &matcher, std::mutex &output_mtx) {
    const auto& root_path = options.root_path;
    const auto& file_extension = options.file_extension;
    for (auto &entry: fs::recursive_directory_iterator(root_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto &path = entry.path();
        if (is_binary_file(path)) {
            continue;
        }
        if (file_extension.has_value() && file_extension.value() != path.extension()) {
            continue;
        }
        pool.submit([path, &matcher, &output_mtx] {
            search_file(path, matcher, output_mtx);
        });
    }
}

void help(const std::string &program_name) {
    std::cerr << "Usage: " << program_name << " <query> <directory> [--regex] [--ignore-case] [--ext=.txt]" <<
        std::endl;
}

std::unique_ptr<IMatcher> create_matcher(const SearchOptions &options) {
    if (options.use_regex) {
        return std::make_unique<RegexMatcher>(options.query, options.ignore_case);
    }
    return std::make_unique<SubstringMatcher>(options.query, options.ignore_case);
}

size_t get_threads_number() {
    constexpr size_t reservedThreads = 2; // reserve 2 threads for other processes.
    auto num_threads = static_cast<size_t>(std::thread::hardware_concurrency());
    num_threads = std::max<size_t>(1, num_threads > reservedThreads ? num_threads - reservedThreads : 1);
    return num_threads;
}

SearchOptions extract_arguments(const int argc, char *argv[]) {
    SearchOptions options {};
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

bool contains_regex_chars(const std::string& query) {
    static const std::regex likely_regex_pattern(R"([.^$*+?{}\[\]\\|()])");
    return std::regex_search(query, likely_regex_pattern);
}

int main(int argc, char *argv[]) {
    constexpr int minimalArgCount = 3;
    if (argc < minimalArgCount) {
        help(argv[0]);
        return 1;
    }
    try {
        auto options = extract_arguments(argc, argv);
        if (!options.use_regex && contains_regex_chars(options.query)) {
            std::cerr << "Warning: The pattern \"" << options.query
                    << "\" looks like a regular expression, but --regex flag was not set.\n";
            return 1;
        }
        auto matcher = create_matcher(options);
        auto num_threads = get_threads_number();
        ThreadPool pool {num_threads};
        std::mutex output_mutex;
        walk_directory(options, pool, *matcher, output_mutex);
    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception!" << std::endl;
    }
    return 0;
}
