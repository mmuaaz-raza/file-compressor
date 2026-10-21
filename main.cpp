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

#include "include/deflate.hpp"
#include "include/huffman.hpp"
#include "include/huffman2.hpp"
#include "include/lzss.hpp"
#include "include/utils.hpp"
#include "include/compressor_interface.hpp"
#include "include/timing_harness.hpp"
#include "include/memory_tracker.hpp"
#include <bits/stdc++.h>

using namespace std;

namespace Color
{
    const string RESET   = "\033[0m";
    const string BOLD    = "\033[1m";
    const string CYAN    = "\033[96m";
    const string YELLOW  = "\033[93m";
    const string GREEN   = "\033[92m";
    const string RED     = "\033[91m";
    const string MAGENTA = "\033[95m";
    const string DIM     = "\033[2m";

    string cyan   (const string &s) { return CYAN    + s + RESET; }
    string yellow (const string &s) { return YELLOW  + s + RESET; }
    string green  (const string &s) { return GREEN   + s + RESET; }
    string red    (const string &s) { return RED     + s + RESET; }
    string magenta(const string &s) { return MAGENTA + s + RESET; }
    string bold   (const string &s) { return BOLD    + s + RESET; }
    string dim    (const string &s) { return DIM     + s + RESET; }
}

static void enableColors()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  mode = 0;
    if (GetConsoleMode(hOut, &mode))
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

static void printSeparator(char ch = '=', int width = 56)
{
    cout << Color::DIM << string(width, ch) << Color::RESET << "\n";
}

static void printHeader()
{
    cout << "\n";
    printSeparator();
    cout << Color::CYAN << Color::BOLD
         << "       FILE COMPRESSOR  --  Algorithm Menu"
         << Color::RESET << "\n";
    printSeparator();
}

static int promptInt(const string &prompt, int lo, int hi)
{
    int choice;
    while (true)
    {
        cout << Color::YELLOW << prompt << Color::RESET;
        if (cin >> choice && choice >= lo && choice <= hi)
            return choice;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << Color::RED << "  [!] Enter a number between "
             << lo << " and " << hi << ".\n" << Color::RESET;
    }
}

static string promptPath(const string &label)
{
    string path;
    cout << Color::DIM << "  " << label << ": " << Color::RESET;
    cin >> path;
    return path;
}

// ── Single-Run CLI Mode ────────────────────────────────────────────────────────
static int handleSingleRun(const string &op, const string &algo, const string &inPath, const string &outPath)
{
    auto comp = createCompressor(algo);
    if (!comp)
    {
        cerr << "Error: Unknown algorithm '" << algo << "'. Available: deflate, huffman, lz77\n";
        return 1;
    }

    string inputStr = readFile(inPath);
    if (inputStr.empty())
    {
        cerr << "Error: Input file is empty or cannot be opened: " << inPath << "\n";
        return 1;
    }

    vector<uint8_t> inBytes(inputStr.begin(), inputStr.end());
    vector<uint8_t> outBytes;

    auto start = chrono::high_resolution_clock::now();
    try
    {
        if (op == "compress")
        {
            outBytes = comp->compress(inBytes);
        }
        else if (op == "decompress")
        {
            outBytes = comp->decompress(inBytes);
        }
        else
        {
            cerr << "Error: Invalid operation '" << op << "'. Use 'compress' or 'decompress'.\n";
            return 1;
        }
    }
    catch (const exception &e)
    {
        cerr << "Exception during " << op << ": " << e.what() << "\n";
        return 1;
    }
    auto end = chrono::high_resolution_clock::now();
    double timeMs = chrono::duration<double, milli>(end - start).count();

    ofstream ofile(outPath, ios::binary);
    if (!ofile)
    {
        cerr << "Error: Could not open output file: " << outPath << "\n";
        return 1;
    }
    ofile.write(reinterpret_cast<const char *>(outBytes.data()), outBytes.size());
    ofile.close();

    size_t peakMem = MemoryTracker::getPeakWorkingSet();

    double ratio = inBytes.empty() ? 0.0 : static_cast<double>(outBytes.size()) / inBytes.size();
    double spaceSavings = (1.0 - ratio) * 100.0;

    cout << "Algorithm     : " << comp->getName() << "\n";
    cout << "Operation     : " << op << "\n";
    cout << "Input Size    : " << inBytes.size() << " bytes\n";
    cout << "Output Size   : " << outBytes.size() << " bytes\n";
    if (op == "compress")
    {
        cout << "Ratio         : " << fixed << setprecision(4) << ratio << "\n";
        cout << "Space Savings : " << fixed << setprecision(2) << spaceSavings << "%\n";
    }
    cout << "Time          : " << TimingHarness::formatTime(timeMs) << "\n";
    cout << "Peak Memory   : " << MemoryTracker::formatBytes(peakMem) << "\n";
    return 0;
}

// ── Automated Benchmark Mode ───────────────────────────────────────────────────
static int handleBenchmark(const string &filePath)
{
    string inputStr = readFile(filePath);
    if (inputStr.empty())
    {
        cerr << "Error: Could not read test file: " << filePath << "\n";
        return 1;
    }

    vector<uint8_t> originalData(inputStr.begin(), inputStr.end());
    size_t origSize = originalData.size();

    cout << "\n";
    printSeparator('=', 76);
    cout << Color::CYAN << Color::BOLD
         << "           BENCHMARK EVALUATION SUITE\n"
         << Color::RESET;
    cout << "  File : " << filePath << " (" << origSize << " bytes)\n";
    printSeparator('=', 76);

    const vector<string> algos = {"huffman", "lz77", "deflate"};

    struct BenchRow
    {
        string algo;
        size_t compressedSize = 0;
        double ratio = 0.0;
        double spaceSavings = 0.0;
        double compTimeMs = 0.0;
        double decompTimeMs = 0.0;
        size_t compPeakMem = 0;
        size_t decompPeakMem = 0;
        bool roundTripOk = false;
    };

    vector<BenchRow> rows;

    for (const auto &algo : algos)
    {
        auto comp = createCompressor(algo);
        if (!comp) continue;

        BenchRow row;
        row.algo = comp->getName();

        // 1. Measure isolated peak memory for compression via fresh child process
        MemoryResult compMem = MemoryTracker::measureIsolated(algo, "compress", filePath);
        row.compPeakMem = compMem.peakBytes;

        // 2. Measure in-memory compression time (warmup + adaptive N)
        vector<uint8_t> compressed;
        TimingResult compTiming = TimingHarness::benchmark([&]() {
            compressed = comp->compress(originalData);
        }, origSize, 0);

        row.compressedSize = compressed.size();
        row.ratio = origSize ? static_cast<double>(row.compressedSize) / origSize : 0.0;
        row.spaceSavings = (1.0 - row.ratio) * 100.0;
        row.compTimeMs = compTiming.meanMs;

        // Save compressed buffer to temp file for child decompress memory measurement
        string tempCompFile = filePath + ".tmp.comp";
        ofstream ofs(tempCompFile, ios::binary);
        ofs.write(reinterpret_cast<const char *>(compressed.data()), compressed.size());
        ofs.close();

        // 3. Measure isolated peak memory for decompression
        MemoryResult decompMem = MemoryTracker::measureIsolated(algo, "decompress", tempCompFile);
        row.decompPeakMem = decompMem.peakBytes;
        remove(tempCompFile.c_str());

        // 4. Measure in-memory decompression time (warmup + adaptive N)
        vector<uint8_t> decompressed;
        TimingResult decompTiming = TimingHarness::benchmark([&]() {
            decompressed = comp->decompress(compressed);
        }, origSize, 0);

        row.decompTimeMs = decompTiming.meanMs;

        // 5. Automatic byte-level round-trip check
        row.roundTripOk = (decompressed == originalData);
        if (!row.roundTripOk)
        {
            cerr << Color::RED << "  [MISMATCH ERROR] Round-trip failed for " << row.algo << "!\n" << Color::RESET;
            continue;
        }

        rows.push_back(row);
    }

    // ── Print CLI Table ────────────────────────────────────────────────────────
    cout << "\n"
         << left << setw(10) << "Algorithm"
         << right << setw(12) << "Comp. Size"
         << right << setw(10) << "Ratio"
         << right << setw(12) << "Savings(%)"
         << right << setw(12) << "Comp. Time"
         << right << setw(12) << "Decomp Time"
         << right << setw(14) << "Peak Mem (C)"
         << right << setw(12) << "Round-Trip"
         << "\n";
    printSeparator('-', 96);

    for (const auto &r : rows)
    {
        cout << left  << setw(10) << r.algo
             << right << setw(12) << r.compressedSize
             << right << setw(10) << fixed << setprecision(4) << r.ratio
             << right << setw(11) << fixed << setprecision(2) << r.spaceSavings << "%"
             << right << setw(12) << TimingHarness::formatTime(r.compTimeMs)
             << right << setw(12) << TimingHarness::formatTime(r.decompTimeMs)
             << right << setw(14) << (to_string(r.compPeakMem / 1024) + " KB")
             << right << setw(12) << (r.roundTripOk ? "OK" : "MISMATCH")
             << "\n";
    }
    printSeparator('=', 96);
    cout << "\n";
    return 0;
}

// ── Interactive Menu Loop ──────────────────────────────────────────────────────
static void runInteractiveMenu()
{
    while (true)
    {
        printHeader();

        cout << "\n"
             << Color::BOLD   << "  Algorithm\n"  << Color::RESET
             << Color::CYAN   << "    [1]" << Color::RESET << "  DEFLATE      "
             << Color::DIM    << "(LZSS + dual Huffman -- best ratio)\n"         << Color::RESET
             << Color::CYAN   << "    [2]" << Color::RESET << "  Huffman v1   "
             << Color::DIM    << "(frequency table in file header)\n"            << Color::RESET
             << Color::CYAN   << "    [3]" << Color::RESET << "  Huffman v2   "
             << Color::DIM    << "(canonical codes + CL meta-compression)\n"     << Color::RESET
             << Color::CYAN   << "    [4]" << Color::RESET << "  LZSS         "
             << Color::DIM    << "(sliding-window only)\n"                       << Color::RESET
             << Color::RED    << "    [0]" << Color::RESET << "  Exit\n\n";

        int algo = promptInt("  Choice [0-4]: ", 0, 4);
        if (algo == 0)
        {
            cout << "\n" << Color::GREEN << "  Goodbye!\n\n" << Color::RESET;
            break;
        }

        cout << "\n"
             << Color::BOLD << "  Operation\n"   << Color::RESET
             << Color::CYAN << "    [1]" << Color::RESET << "  Compress\n"
             << Color::CYAN << "    [2]" << Color::RESET << "  Decompress\n\n";
        int op = promptInt("  Choice [1-2]: ", 1, 2);

        cout << "\n";
        string inFile  = promptPath("Input  file");
        string outFile = promptPath("Output file");

        cout << "\n";
        printSeparator('-');
        const string algoNames[] = {"", "DEFLATE", "Huffman v1", "Huffman v2", "LZSS"};
        cout << Color::DIM  << "  Algorithm : " << Color::RESET
             << Color::MAGENTA << algoNames[algo] << Color::RESET << "\n"
             << Color::DIM  << "  Operation : " << Color::RESET
             << (op == 1 ? Color::GREEN  + "Compress"   + Color::RESET
                         : Color::YELLOW + "Decompress" + Color::RESET) << "\n"
             << Color::DIM  << "  Input     : " << Color::RESET << inFile  << "\n"
             << Color::DIM  << "  Output    : " << Color::RESET << outFile << "\n";
        printSeparator('-');
        cout << "\n";

        bool ok = true;
        if (algo == 1)
            ok = (op == 1) ? compressDeflate(inFile, outFile)
                           : decompressDeflate(inFile, outFile);
        else if (algo == 2)
        {
            if (op == 1) compressHuffman(inFile, outFile);
            else         decompressHuffman(inFile, outFile);
        }
        else if (algo == 3)
        {
            if (op == 1) compressHuffman2(inFile, outFile);
            else         decompressHuffman2(inFile, outFile);
        }
        else
        {
            if (op == 1) compressLZSS(inFile, outFile);
            else         decompressLZSSBin(inFile, outFile);
        }

        cout << "\n";
        if (ok)
        {
            cout << Color::GREEN << "  Done. Output written to: "
                 << Color::RESET << outFile << "\n\n";

            if (op == 1)
            {
                size_t origSize = readFile(inFile).size();
                size_t compSize = readFile(outFile).size();
                double ratio    = origSize ? static_cast<double>(compSize) / origSize : 0.0;
                double savings  = (1.0 - ratio) * 100.0;
                cout << Color::DIM << "  Original     : " << Color::RESET << origSize << " bytes\n"
                     << Color::DIM << "  Compressed   : " << Color::RESET << compSize << " bytes\n"
                     << Color::DIM << "  Ratio        : " << Color::RESET
                     << Color::CYAN << fixed << setprecision(4) << ratio << Color::RESET << "\n"
                     << Color::DIM << "  Space Savings: " << Color::RESET
                     << Color::GREEN << fixed << setprecision(2) << savings << "%" << Color::RESET << "\n";
            }
        }
        else
        {
            cout << Color::RED << "  Operation failed. Check error messages above.\n" << Color::RESET;
        }

        cout << "\n";
        printSeparator();
        cout << Color::DIM << "  Press Enter to continue..." << Color::RESET;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        cout << "\n";
    }
}

// ── Program Entry Point ────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    enableColors();

    // 1. Baseline idle memory query for subprocess measurement
    if (argc == 2 && string(argv[1]) == "--baseline-mem")
    {
        size_t peak = MemoryTracker::getPeakWorkingSet();
        cout << "BASELINE_PEAK_BYTES=" << peak << "\n";
        return 0;
    }

    // 2. Child process memory probe
    if (argc >= 5 && string(argv[1]) == "--measure-mem")
    {
        string algo = argv[2];
        string op   = argv[3];
        string path = argv[4];

        auto comp = createCompressor(algo);
        if (!comp) return 1;

        string dataStr = readFile(path);
        vector<uint8_t> data(dataStr.begin(), dataStr.end());

        if (op == "compress")
        {
            auto out = comp->compress(data);
        }
        else
        {
            auto out = comp->decompress(data);
        }

        size_t peak = MemoryTracker::getPeakWorkingSet();
        cout << "PEAK_MEMORY_BYTES=" << peak << "\n";
        return 0;
    }

    // 3. Single-run CLI: prog <compress|decompress> <algo> <in> <out>
    if (argc >= 5 && (string(argv[1]) == "compress" || string(argv[1]) == "decompress"))
    {
        return handleSingleRun(argv[1], argv[2], argv[3], argv[4]);
    }

    // 4. Automated Benchmark Mode: prog --benchmark <filePath>
    if (argc >= 3 && string(argv[1]) == "--benchmark")
    {
        return handleBenchmark(argv[2]);
    }

    // 5. Default: Interactive Menu
    runInteractiveMenu();
    return 0;
}
