#pragma once
#include <chrono>
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <functional>
#include <string>
#include <iostream>
#include <iomanip>

struct TimingResult
{
    double warmupMs = 0.0;
    std::vector<double> runTimesMs; // individual timed runs in milliseconds
    double meanMs = 0.0;
    double medianMs = 0.0;
    double minMs = 0.0;
    double maxMs = 0.0;
    double stdDevMs = 0.0;
    double throughputMBs = 0.0; // MB/sec based on input bytes and meanMs
    int iterations = 0;
};

class TimingHarness
{
public:
    // Determine number of iterations based on warmup duration if explicitN <= 0
    static int determineAdaptiveN(double warmupSeconds)
    {
        if (warmupSeconds >= 10.0) return 1;
        if (warmupSeconds >= 2.0)  return 2;
        if (warmupSeconds >= 0.5)  return 3;
        if (warmupSeconds >= 0.05) return 5;
        return 10;
    }

    // Measure execution of callable 'fn':
    // 1. One warmup run (not included in average)
    // 2. N timed runs (either explicitly specified, or adaptively determined from warmup)
    // 3. Return detailed timing metrics
    static TimingResult benchmark(
        const std::function<void()> &fn,
        size_t inputBytes = 0,
        int explicitN = 0,
        const std::function<void(int current, int total, double lastMs)> &progressCallback = nullptr)
    {
        using Clock = std::chrono::high_resolution_clock;
        using Duration = std::chrono::duration<double, std::milli>;

        TimingResult result;

        // 1. Warm-up run
        auto t0 = Clock::now();
        fn();
        auto t1 = Clock::now();
        result.warmupMs = Duration(t1 - t0).count();

        // 2. Determine N
        int n = explicitN;
        if (n <= 0)
        {
            n = determineAdaptiveN(result.warmupMs / 1000.0);
        }
        result.iterations = n;
        result.runTimesMs.reserve(n);

        // 3. Timed runs
        for (int i = 0; i < n; ++i)
        {
            auto start = Clock::now();
            fn();
            auto end = Clock::now();
            double ms = Duration(end - start).count();
            result.runTimesMs.push_back(ms);

            if (progressCallback)
            {
                progressCallback(i + 1, n, ms);
            }
        }

        // 4. Compute statistics
        double sum = 0.0;
        result.minMs = result.runTimesMs.front();
        result.maxMs = result.runTimesMs.front();

        for (double ms : result.runTimesMs)
        {
            sum += ms;
            if (ms < result.minMs) result.minMs = ms;
            if (ms > result.maxMs) result.maxMs = ms;
        }

        result.meanMs = sum / n;

        // Median
        std::vector<double> sorted = result.runTimesMs;
        std::sort(sorted.begin(), sorted.end());
        if (n % 2 == 1)
        {
            result.medianMs = sorted[n / 2];
        }
        else
        {
            result.medianMs = (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
        }

        // Standard deviation
        double varianceSum = 0.0;
        for (double ms : result.runTimesMs)
        {
            double diff = ms - result.meanMs;
            varianceSum += diff * diff;
        }
        result.stdDevMs = (n > 1) ? std::sqrt(varianceSum / (n - 1)) : 0.0;

        // Throughput
        if (inputBytes > 0 && result.meanMs > 0.0)
        {
            double inputMB = static_cast<double>(inputBytes) / (1024.0 * 1024.0);
            double seconds = result.meanMs / 1000.0;
            result.throughputMBs = inputMB / seconds;
        }

        return result;
    }

    // Format millisecond duration into human-readable string
    static std::string formatTime(double ms)
    {
        std::ostringstream oss;
        if (ms < 1.0)
        {
            oss << std::fixed << std::setprecision(2) << (ms * 1000.0) << " us";
        }
        else if (ms < 1000.0)
        {
            oss << std::fixed << std::setprecision(2) << ms << " ms";
        }
        else
        {
            oss << std::fixed << std::setprecision(3) << (ms / 1000.0) << " s";
        }
        return oss.str();
    }

    // Pretty-print results
    static void printReport(const std::string &label, const TimingResult &res, size_t inputBytes = 0)
    {
        std::cout << "--- " << label << " ---\n";
        std::cout << "  Warmup run   : " << formatTime(res.warmupMs) << "\n";
        std::cout << "  Iterations   : " << res.iterations << " runs\n";
        std::cout << "  Mean time    : " << formatTime(res.meanMs) << "\n";
        std::cout << "  Median time  : " << formatTime(res.medianMs) << "\n";
        std::cout << "  Min time     : " << formatTime(res.minMs) << "\n";
        std::cout << "  Max time     : " << formatTime(res.maxMs) << "\n";
        std::cout << "  Std Dev      : " << formatTime(res.stdDevMs) << "\n";
        if (inputBytes > 0)
        {
            std::cout << "  Throughput   : " << std::fixed << std::setprecision(2)
                      << res.throughputMBs << " MB/s ("
                      << (inputBytes / 1024.0) << " KB)\n";
        }
        std::cout << "  Run times    : [";
        for (size_t i = 0; i < res.runTimesMs.size(); ++i)
        {
            std::cout << std::fixed << std::setprecision(2) << res.runTimesMs[i] << "ms"
                      << (i + 1 < res.runTimesMs.size() ? ", " : "");
        }
        std::cout << "]\n";
    }
};
