#pragma once
#include <bits/stdc++.h>
#include "utils.hpp"
using namespace std;

// ── LZSS compression (in-memory, absolute offsets) ───────────────────────────
// Compresses `input` into a flag stream and a data stream.
//   flags[i] == true  → match token  → data: (length-3), (distance-1)
//   flags[i] == false → literal token → data: raw byte
void compressLZSSAbs(const string &input,
                     vector<bool>          &flags,
                     vector<unsigned char> &data);

// Decompresses the flag+data streams produced by compressLZSSAbs.
string decompressLZSSAbs(vector<bool>          &flags,
                         vector<unsigned char> &data);

// In-memory compression & decompression
vector<uint8_t> compressLZSSMem  (const vector<uint8_t> &input);
vector<uint8_t> decompressLZSSMem(const vector<uint8_t> &compressed);

// File-level LZSS with bit-packed binary format
void compressLZSS     (const string &inFile, const string &outFile);
void decompressLZSSBin(const string &inFile, const string &outFile);

