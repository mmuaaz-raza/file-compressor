#include "../include/deflate.hpp"
#include "../include/lzss.hpp"
#include "../include/utils.hpp"
#include <cassert>

// ── Tables ────────────────────────────────────────────────────────────────────
const int lengthBase[29]  = {3,4,5,6,7,8,9,10, 11,13,15,17, 19,23,27,31, 35,43,51,59, 67,83,99,115, 131,163,195,227,258};
const int lengthExtra[29] = {0,0,0,0,0,0,0,0,  1,1,1,1,     2,2,2,2,     3,3,3,3,     4,4,4,4,       5,5,5,5,         0};
const int distBase[16]    = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193};
const int distExtra[16]   = {0,0,0,0,1,1,2,2,  3,3,4,4, 5,5,  6,  6};

// ── Bit utilities ─────────────────────────────────────────────────────────────

string deflateIntToBinaryStr(unsigned long long num, int minBits)
{
    string output;
    while (num > 0) { output.push_back((num & 1) ? '1' : '0'); num >>= 1; }
    if (minBits != -1 && output.length() < static_cast<size_t>(minBits))
        output.append(minBits - output.length(), '0');
    reverse(output.begin(), output.end());
    return output;
}

void appendBits(vector<bool> &out, const string &bits)
{
    for (char c : bits) out.push_back(c == '1');
}

// ── DeflateHuffman private ────────────────────────────────────────────────────

void DeflateHuffman::buildCodingLengthTree(Node *n, int depth)
{
    if (!n) return;
    if (!n->left && !n->right)
    {
        assert(n->data >= 0 && n->data < itemSize);
        assert(depth < 256);
        codeLengths[n->data] = (unsigned char)depth;
        return;
    }
    buildCodingLengthTree(n->left,  depth + 1);
    buildCodingLengthTree(n->right, depth + 1);
}

void DeflateHuffman::buildEncodingTable()
{
    encodings.clear();
    vector<pair<uint16_t, int>> items;
    for (int i = 0; i < itemSize; i++)
        if (codeLengths[i]) items.push_back({(uint16_t)i, (int)codeLengths[i]});
    sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
        if (a.second != b.second) return a.second < b.second;
        return a.first < b.first;
    });

    uint32_t code    = 0;
    int      prevLen = 0;
    for (size_t i = 0; i < items.size(); i++)
    {
        int len = items[i].second;
        assert(len <= 31);
        if (i != 0) code++;
        code <<= (len - prevLen);
        prevLen = len;
        string s;
        for (int k = len - 1; k >= 0; k--)
            s.push_back(((code >> k) & 1) ? '1' : '0');
        encodings[items[i].first] = s;
    }
}

void DeflateHuffman::buildDecodingTable()
{
    decodings.clear();
    for (auto &x : encodings) decodings[x.second] = x.first;
}

Node *DeflateHuffman::buildTree(const map<uint16_t, int> &freq)
{
    for (auto &x : freq) queue.enqueue(new Node(x.second, x.first));
    if (queue.getSize() == 1)
    {
        Node *leaf    = queue.dequeue();
        Node *wrapper = new Node(leaf->frequency);
        wrapper->left = leaf;
        return wrapper;
    }
    while (queue.getSize() > 1)
    {
        Node *left  = queue.dequeue();
        Node *right = queue.dequeue();
        Node *sub   = new Node(left->frequency + right->frequency);
        sub->left   = left; sub->right = right;
        queue.enqueue(sub);
    }
    return queue.dequeue();
}

void DeflateHuffman::init()
{
    if (frequencies.empty()) return;
    map<uint16_t, int> freq = frequencies;
    for (int attempt = 0; attempt < 64; attempt++)
    {
        fill(codeLengths.begin(), codeLengths.end(), 0);
        root = buildTree(freq);
        buildCodingLengthTree(root, 0);
        int deepest = *max_element(codeLengths.begin(), codeLengths.end());
        if (deepest <= maxLen) break;
        for (auto &x : freq) x.second = (x.second + 1) / 2;
    }
    assert(*max_element(codeLengths.begin(), codeLengths.end()) <= maxLen);
    buildEncodingTable();
    buildDecodingTable();
}

// ── DeflateHuffman constructors ───────────────────────────────────────────────

DeflateHuffman::DeflateHuffman(const vector<uint16_t> &input, int size, int limit)
{
    itemSize    = size;
    maxLen      = limit;
    codeLengths = vector<unsigned char>(itemSize, 0);
    for (auto x : input) frequencies[x]++;
    init();
}

DeflateHuffman::DeflateHuffman(const vector<unsigned char> &lengths)
{
    codeLengths = lengths;
    itemSize    = (int)lengths.size();
    buildEncodingTable();
    buildDecodingTable();
}

// ── Tokens → symbols ──────────────────────────────────────────────────────────
static void tokensToSymbols(const vector<unsigned char> &data,
                            const vector<bool>          &flags,
                            vector<uint16_t>            &LL,
                            vector<unsigned char>       &LLExtra,
                            vector<uint16_t>            &D,
                            vector<unsigned char>       &DExtra)
{
    LL.clear(); LLExtra.clear(); D.clear(); DExtra.clear();
    size_t j = 0;
    for (bool isMatch : flags)
    {
        if (isMatch)
        {
            int length   = data[j++] + 3;   // 3..258
            int distance = data[j++] + 1;   // 1..256
            int li = (int)(upper_bound(lengthBase, lengthBase + 29, length) - lengthBase) - 1;
            LL.push_back((uint16_t)(256 + li));
            LLExtra.push_back((unsigned char)(length - lengthBase[li]));
            int di = (int)(upper_bound(distBase, distBase + 16, distance) - distBase) - 1;
            D.push_back((uint16_t)di);
            DExtra.push_back((unsigned char)(distance - distBase[di]));
        }
        else
        {
            LL.push_back(data[j++]);
        }
    }
}

// ── Code-length shorthand ─────────────────────────────────────────────────────
static void compressCodeLengthsDeflate(const vector<unsigned char> &codeLengths,
                                       vector<uint16_t>            &compressed,
                                       vector<string>              &extraBits)
{
    int n = (int)codeLengths.size();
    if (!n) return;
    int streak = 1;
    for (int i = 1; i <= n; i++)
    {
        if (i < n && codeLengths[i - 1] == codeLengths[i])
        {
            streak++;
        }
        else
        {
            while (codeLengths[i - 1] == 0 && streak >= 11)
            {
                compressed.push_back(18);
                extraBits.push_back(deflateIntToBinaryStr(min(127, streak - 11), 7));
                streak -= min(138, streak);
            }
            if (streak < 3)
            {
                while (streak--) compressed.push_back(codeLengths[i - 1]);
            }
            else
            {
                if (codeLengths[i - 1] == 0)
                {
                    compressed.push_back(17);
                    extraBits.push_back(deflateIntToBinaryStr(streak - 3, 3));
                }
                else
                {
                    compressed.push_back(codeLengths[i - 1]);
                    streak--;
                    while (streak >= 3)
                    {
                        compressed.push_back(16);
                        extraBits.push_back(deflateIntToBinaryStr(min(3, streak - 3), 2));
                        streak -= min(6, streak);
                    }
                    while (streak--) compressed.push_back(codeLengths[i - 1]);
                }
            }
            streak = 1;
        }
    }
}

// Items → bits (item code, then raw extra bits for 16/17/18)
static vector<bool> encodeCLDeflateItems(const vector<uint16_t>        &items,
                                         const map<uint16_t, string>   &enc,
                                         const vector<string>          &extraBits)
{
    vector<bool> out;
    size_t       j = 0;
    for (uint16_t it : items)
    {
        appendBits(out, enc.at(it));
        if (it > 15) appendBits(out, extraBits.at(j++));
    }
    return out;
}

// Data tokens → bits
static vector<bool> encodeCLDeflateData(const vector<uint16_t>       &LL,
                                        const vector<uint16_t>       &D,
                                        const map<uint16_t, string>  &encLL,
                                        const map<uint16_t, string>  &encD,
                                        const vector<unsigned char>  &LLextra,
                                        const vector<unsigned char>  &Dextra)
{
    vector<bool> out;
    size_t lli = 0, di = 0;
    for (uint16_t sym : LL)
    {
        appendBits(out, encLL.at(sym));
        if (sym > 255)
        {
            appendBits(out, deflateIntToBinaryStr(LLextra[lli++], lengthExtra[sym - 256]));
            uint16_t ds = D[di];
            appendBits(out, encD.at(ds));
            appendBits(out, deflateIntToBinaryStr(Dextra[di], distExtra[ds]));
            di++;
        }
    }
    return out;
}

static void appendBitsToBytes(vector<uint8_t> &out, const vector<bool> &bits)
{
    unsigned char cur = 0;
    int n = 0;
    for (bool b : bits)
    {
        cur = (unsigned char)((cur << 1) | (b ? 1 : 0));
        if (++n == 8) { out.push_back(cur); cur = 0; n = 0; }
    }
    if (n) { cur = (unsigned char)(cur << (8 - n)); out.push_back(cur); }
}

class BitReader
{
    const vector<unsigned char> &bytes;
    size_t pos = 0;

public:
    explicit BitReader(const vector<unsigned char> &b) : bytes(b) {}
    int readBit()
    {
        if (pos >= bytes.size() * 8)
            throw runtime_error("unexpected end of compressed data");
        int bit = (bytes[pos >> 3] >> (7 - (pos & 7))) & 1;
        pos++;
        return bit;
    }
    int readBits(int n)
    {
        int v = 0;
        for (int i = 0; i < n; i++) v = (v << 1) | readBit();
        return v;
    }
};

static uint16_t readSymbol(BitReader &br, const map<string, uint16_t> &table)
{
    string code;
    for (int len = 0; len < MAX_CODE_LEN; len++)
    {
        code.push_back(br.readBit() ? '1' : '0');
        auto it = table.find(code);
        if (it != table.end()) return it->second;
    }
    throw runtime_error("invalid Huffman code");
}

vector<uint8_t> compressDeflateMem(const vector<uint8_t> &input)
{
    string strInput(reinterpret_cast<const char *>(input.data()), input.size());
    long long totalChars = (long long)strInput.length();

    vector<bool> bits;
    if (totalChars > 0)
    {
        vector<unsigned char> LZdata;
        vector<bool>          LZflags;
        compressLZSSAbs(strInput, LZflags, LZdata);

        vector<uint16_t>      LL, D;
        vector<unsigned char> LLExtra, DExtra;
        tokensToSymbols(LZdata, LZflags, LL, LLExtra, D, DExtra);

        DeflateHuffman LLcode(LL, LL_SIZE, MAX_CODE_LEN);
        DeflateHuffman Dcode (D,  D_SIZE,  MAX_CODE_LEN);
        vector<bool> dataBits = encodeCLDeflateData(LL, D,
                                    LLcode.getEncodings(), Dcode.getEncodings(),
                                    LLExtra, DExtra);

        vector<unsigned char> merged = LLcode.getCodeLengths();
        vector<unsigned char> dl     = Dcode.getCodeLengths();
        merged.insert(merged.end(), dl.begin(), dl.end());
        for (auto l : merged)
            if (l > MAX_CODE_LEN)
                throw runtime_error("code length exceeds maximum allowed");

        vector<uint16_t> items;
        vector<string>   extraBits;
        compressCodeLengthsDeflate(merged, items, extraBits);

        DeflateHuffman CLcode(items, CL_SIZE, MAX_ITEM_CODE_LEN);
        vector<unsigned char> clLens = CLcode.getCodeLengths();
        for (auto l : clLens)
            if (l > MAX_ITEM_CODE_LEN)
                throw runtime_error("item code length does not fit in 3 bits");

        for (auto l : clLens) appendBits(bits, deflateIntToBinaryStr(l, 3));
        vector<bool> itemBits = encodeCLDeflateItems(items, CLcode.getEncodings(), extraBits);
        bits.insert(bits.end(), itemBits.begin(), itemBits.end());
        bits.insert(bits.end(), dataBits.begin(), dataBits.end());
    }

    vector<uint8_t> out;
    const uint8_t *tcPtr = reinterpret_cast<const uint8_t *>(&totalChars);
    out.insert(out.end(), tcPtr, tcPtr + sizeof(totalChars));
    appendBitsToBytes(out, bits);
    return out;
}

vector<uint8_t> decompressDeflateMem(const vector<uint8_t> &compressed)
{
    if (compressed.size() < sizeof(long long))
        throw runtime_error("deflate compressed buffer truncated");

    long long totalChars = 0;
    memcpy(&totalChars, compressed.data(), sizeof(totalChars));

    vector<unsigned char> bytes(compressed.begin() + sizeof(totalChars), compressed.end());
    string output;

    if (totalChars > 0)
    {
        BitReader br(bytes);

        vector<unsigned char> clLens;
        for (int i = 0; i < CL_SIZE; i++)
            clLens.push_back((unsigned char)br.readBits(3));
        map<string, uint16_t> clTable = DeflateHuffman(clLens).getDecodings();

        vector<unsigned char> lengths;
        while ((int)lengths.size() < TOTAL_LENGTHS)
        {
            uint16_t item = readSymbol(br, clTable);
            if (item < 16) { lengths.push_back((unsigned char)item); continue; }
            unsigned char value = 0;
            int count;
            if (item == 16)
            {
                if (lengths.empty()) throw runtime_error("corrupt code lengths");
                value = lengths.back();
                count = 3 + br.readBits(2);
            }
            else if (item == 17) count = 3  + br.readBits(3);
            else                 count = 11 + br.readBits(7);
            if ((int)lengths.size() + count > TOTAL_LENGTHS)
                throw runtime_error("corrupt code lengths");
            lengths.insert(lengths.end(), (size_t)count, value);
        }

        vector<unsigned char> LLlens(lengths.begin(), lengths.begin() + LL_SIZE);
        vector<unsigned char> Dlens (lengths.begin() + LL_SIZE, lengths.end());
        map<string, uint16_t> LLtable = DeflateHuffman(LLlens).getDecodings();
        map<string, uint16_t> Dtable  = DeflateHuffman(Dlens).getDecodings();

        while ((long long)output.size() < totalChars)
        {
            uint16_t sym = readSymbol(br, LLtable);
            if (sym < 256) { output.push_back((char)sym); continue; }
            int li = sym - 256;
            if (li >= 29) throw runtime_error("bad length symbol");
            int length = lengthBase[li] + br.readBits(lengthExtra[li]);

            uint16_t ds = readSymbol(br, Dtable);
            if (ds >= D_SIZE) throw runtime_error("bad distance symbol");
            int distance = distBase[ds] + br.readBits(distExtra[ds]);
            if ((size_t)distance > output.size())
                throw runtime_error("distance reaches before start of output");

            size_t start = output.size() - distance;
            for (int k = 0; k < length; k++)
                output.push_back(output[start + k]);
        }
        if ((long long)output.size() != totalChars)
            throw runtime_error("output size mismatch");
    }

    return vector<uint8_t>(output.begin(), output.end());
}

bool compressDeflate(const string &inFile, const string &outFile)
{
    string input = readFile(inFile);
    vector<uint8_t> inBytes(input.begin(), input.end());
    try
    {
        vector<uint8_t> comp = compressDeflateMem(inBytes);
        ofstream file(outFile, ios::binary);
        if (!file) { cerr << "Error: Could not open file " << outFile << endl; return false; }
        file.write(reinterpret_cast<const char *>(comp.data()), comp.size());
        return !file.fail();
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
}

bool decompressDeflate(const string &inFile, const string &outFile)
{
    string input = readFile(inFile);
    vector<uint8_t> inBytes(input.begin(), input.end());
    try
    {
        vector<uint8_t> decomp = decompressDeflateMem(inBytes);
        ofstream file(outFile, ios::binary);
        if (!file) { cerr << "Error: Could not open file " << outFile << endl; return false; }
        file.write(reinterpret_cast<const char *>(decomp.data()), decomp.size());
        return !file.fail();
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
}

