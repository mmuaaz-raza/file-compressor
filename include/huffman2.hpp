#pragma once
#include <bits/stdc++.h>
#include "priority_queue.hpp"
using namespace std;

// ── Utilities shared by huffman2 and deflate ──────────────────────────────────

// MSB-first binary string, padded to minBits (num==0 → all zeros).
string intToBinaryStr(unsigned long long num, int minBits = -1);

// Convert a binary string (up to 8 chars) back to an unsigned char.
unsigned char binaryToNumber(const string &binary);

// ── HuffmanCode (v2) ─────────────────────────────────────────────────────────
// Canonical Huffman with code-length compression (symbols: unsigned char).
class HuffmanCode2
{
    Node         *root         = nullptr;
    int           itemSize     = 256;
    PriorityQueue queue;
    int           totalInputLen = 0;
    string        InputContent;
    map<unsigned char, string>  encodings;
    map<string, unsigned char>  decodings;
    vector<unsigned char>       codeLengths;

    void buildCodingLengthTree(Node *root, unsigned char codeLen);
    void buildDecodingTable();
    void buildFrequencyTable(const string &input);
    void buildFrequencyTable(const vector<unsigned char> &input);
    void init();
    void buildEncodingTable();

public:
    map<unsigned char, int> frequencies;

    explicit HuffmanCode2(const string &input);
    HuffmanCode2(const vector<unsigned char> &input, int size);
    explicit HuffmanCode2(const vector<unsigned char> &codeLengths);

    vector<unsigned char>       getCodeLengths() const { return codeLengths; }
    map<unsigned char, string>  getEncodings();
    map<string, unsigned char>  getDecodings();
    vector<bool>                encode();
    string                      decode(vector<bool> &encodedStr);
};

// ── Code-length run-length helpers ───────────────────────────────────────────
void compressCodeLengths(const vector<unsigned char> &codeLengths,
                         vector<unsigned char>       &compressed,
                         vector<string>              &extraBits);

void decompressCodeLengths(const vector<unsigned char> &compressed,
                           const vector<unsigned char> &extraBits,
                           vector<unsigned char>       &codeLengths);

// Encode compressed code-length items into bits.
vector<bool> encodeCL(vector<unsigned char>            &compressedCodeLengths,
                      map<unsigned char, string>        &encodings,
                      vector<string>                    &extraBits);

// In-memory compression & decompression
vector<uint8_t> compressHuffman2Mem  (const vector<uint8_t> &input);
vector<uint8_t> decompressHuffman2Mem(const vector<uint8_t> &compressed);

// File-level wrappers
void compressHuffman2  (const string &inFile, const string &outFile);
void decompressHuffman2(const string &inFile, const string &outFile);

