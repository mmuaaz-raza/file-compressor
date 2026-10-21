#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

class ICompressor
{
public:
    virtual ~ICompressor() = default;
    virtual std::string getName() const = 0;
    virtual std::vector<uint8_t> compress(const std::vector<uint8_t> &input) = 0;
    virtual std::vector<uint8_t> decompress(const std::vector<uint8_t> &compressed) = 0;
};

std::unique_ptr<ICompressor> createCompressor(const std::string &name);
