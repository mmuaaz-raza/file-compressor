#include "../include/compressor_interface.hpp"
#include "../include/deflate.hpp"
#include "../include/huffman2.hpp"
#include "../include/lzss.hpp"
#include <algorithm>

namespace
{
    class DeflateCompressor : public ICompressor
    {
    public:
        std::string getName() const override { return "DEFLATE"; }

        std::vector<uint8_t> compress(const std::vector<uint8_t> &input) override
        {
            return compressDeflateMem(input);
        }

        std::vector<uint8_t> decompress(const std::vector<uint8_t> &compressed) override
        {
            return decompressDeflateMem(compressed);
        }
    };

    class HuffmanCompressor : public ICompressor
    {
    public:
        std::string getName() const override { return "Huffman"; }

        std::vector<uint8_t> compress(const std::vector<uint8_t> &input) override
        {
            return compressHuffman2Mem(input);
        }

        std::vector<uint8_t> decompress(const std::vector<uint8_t> &compressed) override
        {
            return decompressHuffman2Mem(compressed);
        }
    };

    class LZSSCompressor : public ICompressor
    {
    public:
        std::string getName() const override { return "LZ77"; }

        std::vector<uint8_t> compress(const std::vector<uint8_t> &input) override
        {
            return compressLZSSMem(input);
        }

        std::vector<uint8_t> decompress(const std::vector<uint8_t> &compressed) override
        {
            return decompressLZSSMem(compressed);
        }
    };
}

std::unique_ptr<ICompressor> createCompressor(const std::string &name)
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "deflate")
        return std::make_unique<DeflateCompressor>();
    if (lower == "huffman" || lower == "huffman2")
        return std::make_unique<HuffmanCompressor>();
    if (lower == "lzss" || lower == "lz77")
        return std::make_unique<LZSSCompressor>();

    return nullptr;
}
