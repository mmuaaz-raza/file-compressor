#pragma once
#include <bits/stdc++.h>
#include "priority_queue.hpp"
using namespace std;

// ── HuffmanCode (v1) ─────────────────────────────────────────────────────────
// Simple Huffman code for byte (unsigned char) symbols.
// Frequency table is stored in the compressed file header.
class HuffmanCode
{
    Node         *root          = nullptr;
    PriorityQueue queue;
    int           totalInputLen = 0;
    string        InputContent;
    map<unsigned char, string>  encodings;
    map<string, unsigned char>  decodings;

    void buildEncodingTable(Node *root, const string &code);
    void buildDecodingTable();
    float getEntorpy(Node *root);
    void  buildFrequencyTable(const string &input);
    void  init(map<unsigned char, int> frequencies);
    void  init();

public:
    map<unsigned char, int> frequencies;

    explicit HuffmanCode(const string &input);
    explicit HuffmanCode(map<unsigned char, int> frequencies);

    map<unsigned char, string>  getEncodings();
    map<string, unsigned char>  getDecodings();
    float                       getEntropy();
    vector<bool>                originalBin();
    vector<bool>                encode();
    string                      decode(vector<bool> &encodedStr);
};

// ── File-level compress / decompress ─────────────────────────────────────────
void compressHuffman  (const string &filePath, const string &outFile);
void decompressHuffman(const string &inputFile, const string &outFile);
