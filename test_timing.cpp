#include "include/compressor_interface.hpp"
#include "include/timing_harness.hpp"
#include "include/utils.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

static void evaluateCompressor(const std::string &algoName, const std::vector<uint8_t> &data)
{
    std::cout << "=====================================================\n";
    std::cout << " Testing Compressor: " << algoName << "\n";
    std::cout << "=====================================================\n";

    auto comp = createCompressor(algoName);
    assert(comp != nullptr);

    // Warmup & benchmark in-memory compression
    std::vector<uint8_t> compressed;
    auto compressTiming = TimingHarness::benchmark([&]() {
        compressed = comp->compress(data);
    }, data.size(), 0);

    TimingHarness::printReport(algoName + " - Compression", compressTiming, data.size());

    double ratio = data.empty() ? 0.0 : static_cast<double>(compressed.size()) / data.size();
    double spaceSavings = (1.0 - ratio) * 100.0;

    std::cout << "  Original size: " << data.size() << " bytes\n";
    std::cout << "  Compressed   : " << compressed.size() << " bytes\n";
    std::cout << "  Ratio        : " << std::fixed << std::setprecision(4) << ratio << "\n";
    std::cout << "  Space Savings: " << std::fixed << std::setprecision(2) << spaceSavings << "%\n\n";

    // Warmup & benchmark in-memory decompression
    std::vector<uint8_t> decompressed;
    auto decompressTiming = TimingHarness::benchmark([&]() {
        decompressed = comp->decompress(compressed);
    }, data.size(), 0);

    TimingHarness::printReport(algoName + " - Decompression", decompressTiming, data.size());

    // Round-trip verification
    bool match = (decompressed == data);
    std::cout << "  Round-trip byte match: " << (match ? "VERIFIED OK" : "FAILED (MISMATCH!)") << "\n\n";
    assert(match);
}

int main()
{
    std::cout << "--- Common In-Memory Interface & Timing Evaluation ---\n\n";

    std::string text = "Antigravity Compression Engine: Testing the common interface. "
                       "This is repetitive text used for in-memory round trip verification! ";
    for (int i = 0; i < 5; ++i) text += text;

    std::vector<uint8_t> sampleData(text.begin(), text.end());

    evaluateCompressor("huffman", sampleData);
    evaluateCompressor("lz77",    sampleData);
    evaluateCompressor("deflate", sampleData);

    std::cout << "ALL THREE MODULES ROUND-TRIP VERIFIED VIA ICompressor.\n";
    return 0;
}
