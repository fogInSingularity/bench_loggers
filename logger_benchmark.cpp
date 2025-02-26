#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <thread>
#include <memory>

// --- Logger Library Headers ---
#include "spdlog/async_logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "quill/core/PatternFormatterOptions.h"
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/Logger.h"
#include "quill/sinks/FileSink.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

// --- Abstract Logger Interface ---

class Logger {
public:
    virtual ~Logger() = default;
    virtual void init() = 0;
    virtual void log_message(const std::string& message) = 0;
    virtual std::string get_name() const = 0;
    virtual void shutdown() = 0;
};

// --- Spdlog Logger Implementation ---
class SpdlogLogger : public Logger {
private:
    std::shared_ptr<spdlog::logger> logger_;
    std::string logger_name_ = "Spdlog";
    std::string log_file_path_;

public:
    SpdlogLogger(const std::string& log_file_path = "spdlog_benchmark.log")
      : log_file_path_(log_file_path) {}

    void init() override {
        try {
            std::string log_pattern{"%v"};
            // size_t thread_pool_queue_size = 1024 * 1024;
            // spdlog::init_thread_pool(thread_pool_queue_size, 1);
            // if used with one thread: mt -> st
            logger_ = spdlog::basic_logger_st("file_logger", log_file_path_, true);
            // logger_ = spdlog::basic_logger_mt<spdlog::async_factory>("file_logger", log_file_path_, true);
            logger_->set_pattern(log_pattern);
        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Spdlog initialization failed: " << ex.what() << std::endl;
            throw;
        }
    }

    void log_message(const std::string& message) override {
        SPDLOG_LOGGER_INFO(logger_, "{}", message);
    }

    std::string get_name() const override {
        return logger_name_;
    }

    void shutdown() override {
        spdlog::shutdown();
    }
};

// --- Quill Logger Implementation ---
class QuillLogger : public Logger {
private:
    quill::Logger* logger_;
    std::string logger_name_ = "Quill";
    std::string log_file_path_;

public:
    QuillLogger(const std::string& log_file_path = "quill_benchmark.log")
      : log_file_path_(log_file_path), logger_(nullptr) {}

    void init() override {
        std::string log_pattern{"%(message)"};
        // Start Quill’s backend logging thread (new API)
        // quill::detail::set_cpu_affinity(1);
        quill::Backend::start();

        auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(log_file_path_);
        logger_ = quill::Frontend::create_or_get_logger("file_logger", 
                                                        std::move(file_sink),
                                                        quill::PatternFormatterOptions{log_pattern}
                                                        );
    }

    void log_message(const std::string& message) override {
        QUILL_LOG_INFO(logger_, "{}", message);
    }

    std::string get_name() const override {
        return logger_name_;
    }

    void shutdown() override {
        logger_->flush_log(0);
    }
};

using resolution_t = std::chrono::milliseconds;
// using resolution_t = std::chrono::microseconds;

// --- Benchmark Function ---
void run_benchmark(Logger& logger, int num_messages, int num_threads, const std::string& message_payload) {
    std::vector<std::thread> threads;

    resolution_t time_sum{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&time_sum, &logger, num_messages, num_threads, i, &message_payload]() {
            auto start_time = std::chrono::high_resolution_clock::now();
            int messages_per_thread = num_messages / num_threads;
            for (int j = 0; j < messages_per_thread; ++j) {
                logger.log_message(message_payload);
            }
            // Last thread handles any remainder messages
            if (i == num_threads - 1) {
                for (int j = 0; j < num_messages % num_threads; ++j) {
                    logger.log_message(message_payload);
                }
            }
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<resolution_t>(end_time - start_time);
            time_sum += duration;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // double messages_per_sec = (static_cast<double>(num_messages) / duration.count()) * 1000.0;

    std::cout << std::left << std::setw(10) << logger.get_name()
              << std::setw(15) << num_threads
              << std::setw(15) << num_messages
              << std::setw(20) << time_sum.count()
              // << std::fixed << std::setprecision(2) << std::setw(15) << messages_per_sec
              << std::endl;
}

int main(const int argc, const char** argv) {
    // max 10'000'000
    if (argc != 3) {
        std::cerr << "Error: Incorrect number of arguments." << std::endl;
        std::cout << "Usage: " << argv[0] << " <num_threads> <num_messages>" << std::endl;
        return 1;
    }

    int num_threads = std::atoi(argv[1]);
    int num_messages = std::atoi(argv[2]);

    if (num_threads <= 0) {
        std::cerr << "Error: Number of threads must be a positive integer." << std::endl;
        return 1;
    }

    if (num_messages <= 0) {
        std::cerr << "Error: Number of messages must be a positive integer." << std::endl;
        return 1;
    }

    std::string message_payload = "Benchmark message: This is a test log message to measure logger performance.";

    std::cout << "--- Logger Benchmarking ---" << std::endl;
    std::cout << std::left << std::setw(10) << "Logger"
              << std::setw(15) << "Threads"
              << std::setw(15) << "Messages"
              << std::setw(20) << "Duration (ms)"
              // << std::setw(20) << "Duration (mcs)"
              << std::setw(15) << "Msg/sec"
              << std::endl;
    std::cout << std::string(75, '-') << std::endl;

    std::vector<std::unique_ptr<Logger>> loggers;
    loggers.push_back(std::make_unique<SpdlogLogger>());
    loggers.push_back(std::make_unique<QuillLogger>());

    for (auto& logger : loggers) {
        try {
            logger->init();
            run_benchmark(*logger, num_messages, num_threads, message_payload);
            logger->shutdown();
        } catch (const std::exception& ex) {
            std::cerr << "Benchmark failed for " << logger->get_name() << ": " << ex.what() << std::endl;
        }
    }

    std::cout << "--- Benchmarking Complete ---" << std::endl;

    return 0;
}

