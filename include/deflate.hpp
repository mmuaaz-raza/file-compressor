#pragma once
#include <bits/stdc++.h>
#include "priority_queue.hpp"
using namespace std;

// ── Alphabet constants ────────────────────────────────────────────────────────
constexpr int LL_SIZE          = 285; // 0-255 literals, 256-284 length symbols
constexpr int D_SIZE           = 16;  // distance symbols 0-15 (window 256)
constexpr int CL_SIZE          = 19;  // code-length items 0-18
constexpr int TOTAL_LENGTHS    = LL_SIZE + D_SIZE; // 301
constexpr int MAX_CODE_LEN     = 15;
constexpr int MAX_ITEM_CODE_LEN = 7;

extern const int lengthBase[29];
extern const int lengthExtra[29];
extern const int distBase[16];
extern const int distExtra[16];

// ── Bit utilities ─────────────────────────────────────────────────────────────
// MSB-first binary string, padded to minBits; num==0 gives minBits zeros.
string deflateIntToBinaryStr(unsigned long long num, int minBits = -1);
void   appendBits(vector<bool> &out, const string &bits);

// ── HuffmanCode (deflate-internal, widened to uint16_t symbols) ───────────────
class DeflateHuffman
{
    Node         *root     = nullptr;
    int           itemSize = 256;
    int           maxLen   = 15;
    PriorityQueue queue;
    map<uint16_t, string>  encodings;
    map<string, uint16_t>  decodings;
    vector<unsigned char>  codeLengths;

    void buildCodingLengthTree(Node *n, int depth);
    void buildEncodingTable();
    void buildDecodingTable();
    Node *buildTree(const map<uint16_t, int> &freq);
    void init();

public:
    map<uint16_t, int> frequencies;

    // Encoder side: count symbols, build limited-length codes.
    DeflateHuffman(const vector<uint16_t> &input, int size, int limit = 15);
    // Decoder side: rebuild codes from stored code lengths.
    explicit DeflateHuffman(const vector<unsigned char> &lengths);

    vector<unsigned char>  getCodeLengths() const { return codeLengths; }
    map<uint16_t, string>  getEncodings()         { return encodings;   }
    map<string, uint16_t>  getDecodings()         { return decodings;   }
};

// In-memory compression & decompression
vector<uint8_t> compressDeflateMem  (const vector<uint8_t> &input);
vector<uint8_t> decompressDeflateMem(const vector<uint8_t> &compressed);

// File-level wrappers
bool compressDeflate  (const string &inFile, const string &outFile);
bool decompressDeflate(const string &inFile, const string &outFile);

