#include <iostream>
#include <filesystem>
#include <regex>
#include <fstream>
#include <thread>

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

template<typename Type>
class ThreadSafeQueue {
    std::queue<Type> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic_bool done_;

public:
    void push(const Type &value) {
        std::lock_guard lock(mutex_);
        queue_.push(value);
        cv_.notify_one();
    }

    bool pop(Type &value) {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || done_; });
        if (queue_.empty()) {
            return false;
        }
        value = queue_.front();
        queue_.pop();
        return true;
    }

    void done() {
        done_ = true;
        cv_.notify_all();
    }
};

void worker(ThreadSafeQueue<fs::path> &file_queue, const IMatcher &matcher, std::mutex &output_mutex) {
    fs::path path;
    while (file_queue.pop(path)) {
        std::ifstream file(path);
        if (!file) {
            continue;
        }
        std::string line;
        size_t line_num = 0;
        while (std::getline(file, line)) {
            ++line_num;
            if (matcher.match(line)) {
                std::lock_guard lock(output_mutex);
                std::cout << path << ": " << line_num << ": " << line << std::endl;
            }
        }
    }
}

void walk_directory(const fs::path &root_path, ThreadSafeQueue<fs::path> &file_queue, const SearchOptions &options) {
    for (auto &entry: fs::recursive_directory_iterator(root_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto &path = entry.path();
        if (is_binary_file(path)) {
            continue;
        }
        if (options.file_extension.has_value() && options.file_extension.value() != path.extension()) {
            continue;
        }
        file_queue.push(path);
    }
    file_queue.done();
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

int main(int argc, char *argv[]) {
    constexpr int minimalArgCount = 3;
    if (argc < minimalArgCount) {
        help(argv[0]);
        return 1;
    }
    try {
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
        auto matcher = create_matcher(options);
        ThreadSafeQueue<fs::path> file_queue;
        std::mutex output_mutex;
        auto num_threads = static_cast<size_t>(std::thread::hardware_concurrency());
        constexpr size_t reservedThreads = 2; // reserve two threads for the system execution
        if (num_threads > reservedThreads) {
            num_threads -= reservedThreads;
        }
        std::vector<std::thread> workers;
        workers.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back(worker, std::ref(file_queue), std::cref(*matcher), std::ref(output_mutex));
        }
        walk_directory(options.root_path, file_queue, options);
        for (auto &worker: workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception!" << std::endl;
    }

    return 0;
}
