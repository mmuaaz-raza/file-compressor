#pragma once
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif
#include <cstddef>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <memory>
#include <array>

struct MemoryResult
{
    size_t baselineBytes = 0;
    size_t peakBytes = 0;
    size_t netBytes = 0;
};

class MemoryTracker
{
public:
    static std::string getExecutablePath()
    {
#ifdef _WIN32
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
        if (len > 0) return std::string(buf, len);
#endif
        return "deflate_app.exe";
    }

    static size_t getPeakWorkingSet()
    {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            return static_cast<size_t>(pmc.PeakWorkingSetSize);
        }
#endif
        return 0;
    }

    static size_t getCurrentWorkingSet()
    {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            return static_cast<size_t>(pmc.WorkingSetSize);
        }
#endif
        return 0;
    }

    static std::string formatBytes(size_t bytes)
    {
        std::ostringstream oss;
        double kb = static_cast<double>(bytes) / 1024.0;
        double mb = kb / 1024.0;

        if (mb >= 1.0)
        {
            oss << std::fixed << std::setprecision(2) << mb << " MB (" << bytes << " bytes)";
        }
        else
        {
            oss << std::fixed << std::setprecision(2) << kb << " KB (" << bytes << " bytes)";
        }
        return oss.str();
    }

    static MemoryResult measureIsolated(
        const std::string &algo,
        const std::string &op,
        const std::string &filePath)
    {
        MemoryResult result;
        std::string exePath = getExecutablePath();

        // 1. Measure baseline idle process
        std::string baseCmd = "\"\"" + exePath + "\" --baseline-mem\"";
        FILE *pipeBase = _popen(baseCmd.c_str(), "r");
        if (pipeBase)
        {
            char buf[256];
            while (fgets(buf, sizeof(buf), pipeBase))
            {
                std::string line(buf);
                if (line.rfind("BASELINE_PEAK_BYTES=", 0) == 0)
                {
                    result.baselineBytes = std::stoull(line.substr(20));
                }
            }
            _pclose(pipeBase);
        }

        // 2. Measure child process performing the operation
        std::string runCmd = "\"\"" + exePath + "\" --measure-mem " + algo + " " + op + " \"" + filePath + "\"\"";
        FILE *pipeRun = _popen(runCmd.c_str(), "r");
        if (pipeRun)
        {
            char buf[256];
            while (fgets(buf, sizeof(buf), pipeRun))
            {
                std::string line(buf);
                if (line.rfind("PEAK_MEMORY_BYTES=", 0) == 0)
                {
                    result.peakBytes = std::stoull(line.substr(18));
                }
            }
            _pclose(pipeRun);
        }

        if (result.peakBytes > result.baselineBytes)
        {
            result.netBytes = result.peakBytes - result.baselineBytes;
        }
        else
        {
            result.netBytes = 0;
        }

        return result;
    }
};
