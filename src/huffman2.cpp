#include "../include/huffman2.hpp"
#include "../include/utils.hpp"

// ── intToBinaryStr / binaryToNumber ──────────────────────────────────────────

string intToBinaryStr(unsigned long long num, int minBits)
{
    string output;
    if (num == 0)
        output = "0";
    else
        while (num > 0) { output.push_back((num & 1) ? '1' : '0'); num >>= 1; }

    if (minBits != -1 && output.length() < static_cast<size_t>(minBits))
        output.append(minBits - output.length(), '0');
    reverse(output.begin(), output.end());
    return output;
}

unsigned char binaryToNumber(const string &binary)
{
    int n = (int)binary.size();
    if (n > 8) { cout << "out of range binary input\n"; return 0; }
    unsigned num = 0;
    for (auto x : binary) { num <<= 1; if (x == '1') num |= 1; }
    return (unsigned char)num;
}

// ── HuffmanCode2 private ──────────────────────────────────────────────────────

void HuffmanCode2::buildCodingLengthTree(Node *root, unsigned char codeLen)
{
    if (!root) return;
    if (!root->left && !root->right) { codeLengths[root->data] = codeLen; return; }
    if (root->left)  buildCodingLengthTree(root->left,  codeLen + 1);
    if (root->right) buildCodingLengthTree(root->right, codeLen + 1);
}

void HuffmanCode2::buildDecodingTable()
{
    if (decodings.size()) return;
    for (auto x : encodings) decodings[x.second] = x.first;
}

void HuffmanCode2::buildFrequencyTable(const string &input)
{
    InputContent  = input;
    totalInputLen = (int)input.length();
    for (auto x : input) frequencies[(unsigned char)x]++;
}

void HuffmanCode2::buildFrequencyTable(const vector<unsigned char> &input)
{
    totalInputLen = (int)input.size();
    for (auto x : input) frequencies[x]++;
}

void HuffmanCode2::init()
{
    if (frequencies.empty()) return;

    map<unsigned char, int> freq = frequencies;
    for (int attempt = 0; attempt < 64; attempt++)
    {
        fill(codeLengths.begin(), codeLengths.end(), 0);

        for (auto x : freq)
            queue.enqueue(new Node(x.second, x.first));

        if (queue.getSize() == 1)
        {
            Node *leaf = queue.dequeue();
            Node *wrapper = new Node(leaf->frequency);
            wrapper->left = leaf;
            root = wrapper;
        }
        else
        {
            while (queue.getSize() > 1)
            {
                Node *left  = queue.dequeue();
                Node *right = queue.dequeue();
                Node *subRoot  = new Node(left->frequency + right->frequency);
                subRoot->left  = left;
                subRoot->right = right;
                queue.enqueue(subRoot);
            }
            root = queue.dequeue();
        }

        buildCodingLengthTree(root, 0);
        int deepest = *max_element(codeLengths.begin(), codeLengths.end());
        if (deepest <= 15) break;

        for (auto &x : freq)
            x.second = (x.second + 1) / 2;
    }

    buildEncodingTable();
    buildDecodingTable();
}

void HuffmanCode2::buildEncodingTable()
{
    vector<pair<unsigned char, unsigned char>> codeLengthsMap;
    for (int i = 0; i < itemSize; i++)
        if (codeLengths[i])
            codeLengthsMap.push_back({(unsigned char)i, codeLengths[i]});

    sort(codeLengthsMap.begin(), codeLengthsMap.end(), [](auto &a, auto &b) {
        if (a.second == b.second) return a.first < b.first;
        return a.second < b.second;
    });

    int  code = 0;
    int  j    = 0;
    for (int i = 0; i < (int)codeLengthsMap.size(); i++)
    {
        string codeString;
        if (i != 0) code += 1;
        while (j < codeLengthsMap[i].second) { code <<= 1; j++; }
        for (int k = j - 1; k >= 0; k--)
            codeString.push_back((code & (1 << k)) ? '1' : '0');
        encodings[codeLengthsMap[i].first] = codeString;
    }
}

// ── HuffmanCode2 constructors ─────────────────────────────────────────────────

HuffmanCode2::HuffmanCode2(const string &input)
{
    codeLengths = vector<unsigned char>(itemSize, 0);
    buildFrequencyTable(input);
    init();
}

HuffmanCode2::HuffmanCode2(const vector<unsigned char> &input, int size)
{
    itemSize    = size;
    codeLengths = vector<unsigned char>(itemSize, 0);
    buildFrequencyTable(input);
    init();
}

HuffmanCode2::HuffmanCode2(const vector<unsigned char> &lengths)
{
    codeLengths = lengths;
    itemSize    = (int)lengths.size();
    buildEncodingTable();
    buildDecodingTable();
}

// ── Public accessors ──────────────────────────────────────────────────────────

map<unsigned char, string> HuffmanCode2::getEncodings() { return encodings; }
map<string, unsigned char> HuffmanCode2::getDecodings() { return decodings; }

vector<bool> HuffmanCode2::encode()
{
    vector<bool> compressed;
    string       output;
    for (auto x : InputContent) output += encodings[(unsigned char)x];
    for (auto x : output)       compressed.push_back(x - '0');
    return compressed;
}

string HuffmanCode2::decode(vector<bool> &encodedStr)
{
    string output, temp;
    for (auto x : encodedStr)
    {
        temp.push_back(x ? '1' : '0');
        if (decodings.count(temp)) { output.push_back(decodings[temp]); temp = ""; }
    }
    return output;
}

// ── compressCodeLengths ───────────────────────────────────────────────────────

void compressCodeLengths(const vector<unsigned char> &codeLengths,
                         vector<unsigned char>       &compressed,
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
                compressed.push_back((unsigned char)18);
                extraBits.push_back(intToBinaryStr(min(127, streak - 11), 7));
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
                    compressed.push_back((unsigned char)17);
                    extraBits.push_back(intToBinaryStr(streak - 3, 3));
                }
                else
                {
                    compressed.push_back(codeLengths[i - 1]);
                    streak--;
                    while (streak >= 3)
                    {
                        compressed.push_back((unsigned char)16);
                        extraBits.push_back(intToBinaryStr(min(3, streak - 3), 2));
                        streak -= min(6, streak);
                    }
                    while (streak--) compressed.push_back(codeLengths[i - 1]);
                }
            }
            streak = 1;
        }
    }
}

// ── decompressCodeLengths ─────────────────────────────────────────────────────

void decompressCodeLengths(const vector<unsigned char> &compressed,
                           const vector<unsigned char> &extraBits,
                           vector<unsigned char>       &codeLengths)
{
    int n = (int)compressed.size();
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        if (compressed[i] > 15)
        {
            if (compressed[i] == 16)
            {
                int count = extraBits[j++] + 3;
                unsigned char lastPushed = codeLengths.back();
                while (count--) codeLengths.push_back(lastPushed);
            }
            if (compressed[i] == 17)
            {
                int count = extraBits[j++] + 3;
                while (count--) codeLengths.push_back(0);
            }
            if (compressed[i] == 18)
            {
                int count = extraBits[j++] + 11;
                while (count--) codeLengths.push_back(0);
            }
        }
        else
        {
            codeLengths.push_back(compressed[i]);
        }
    }
}

// ── encodeCL ─────────────────────────────────────────────────────────────────

vector<bool> encodeCL(vector<unsigned char>     &compressedCodeLengths,
                      map<unsigned char, string> &encodings,
                      vector<string>             &extraBits)
{
    vector<bool> compressed;
    int          j = 0;
    for (auto x : compressedCodeLengths)
    {
        for (auto c : encodings[x]) compressed.push_back(c - '0');
        if (x > 15)
            for (auto c : extraBits[j++]) compressed.push_back(c - '0');
    }
    return compressed;
}

// ── compressHuffman2 ──────────────────────────────────────────────────────────

vector<uint8_t> compressHuffman2Mem(const vector<uint8_t> &input)
{
    string strInput(reinterpret_cast<const char *>(input.data()), input.size());
    long long totalChars = (long long)strInput.length();

    vector<uint8_t> out;
    if (totalChars == 0)
    {
        unsigned char totalCompressedCL = 0;
        const uint8_t *tcPtr = reinterpret_cast<const uint8_t *>(&totalChars);
        out.insert(out.end(), tcPtr, tcPtr + sizeof(totalChars));
        out.push_back(totalCompressedCL);
        out.insert(out.end(), 8, 0);
        return out;
    }

    HuffmanCode2 compressor(strInput);
    vector<unsigned char> codeLengths    = compressor.getCodeLengths();
    vector<bool>          compressedData = compressor.encode();

    vector<unsigned char> compactCL;
    vector<string>        extraBitsCL;
    compressCodeLengths(codeLengths, compactCL, extraBitsCL);

    HuffmanCode2 compressorCL(compactCL, 19);
    map<unsigned char, string> encodings = compressorCL.getEncodings();
    vector<bool> compressedCodeLengths   = encodeCL(compactCL, encodings, extraBitsCL);

    unsigned char totalCompressedCL = (unsigned char)compactCL.size();

    const uint8_t *tcPtr = reinterpret_cast<const uint8_t *>(&totalChars);
    out.insert(out.end(), tcPtr, tcPtr + sizeof(totalChars));
    out.push_back(totalCompressedCL);

    vector<unsigned char> compressedCL = compressorCL.getCodeLengths();
    vector<bool> clBits;
    for (int i = 0; i < 19; i++)
    {
        unsigned char len = (i < (int)compressedCL.size()) ? (compressedCL[i] & 7) : 0;
        string bStr = intToBinaryStr(len, 3);
        for (char c : bStr) clBits.push_back(c == '1');
    }
    while (clBits.size() < 64) clBits.push_back(false);
    for (size_t i = 0; i < 64; i += 8)
    {
        uint8_t byte = 0;
        for (int b = 0; b < 8; b++)
            if (clBits[i + b]) byte |= (1 << (7 - b));
        out.push_back(byte);
    }

    char temp = 0;
    int  n    = (int)compressedCodeLengths.size();
    for (int i = 0; i < n; i += 8)
    {
        unsigned int t2 = 1 << 7;
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) >= n) break;
            if (compressedCodeLengths[i + j]) temp |= t2;
            t2 >>= 1;
        }
        out.push_back(static_cast<uint8_t>(temp));
        temp = 0;
    }

    temp = 0;
    n    = (int)compressedData.size();
    for (int i = 0; i < n; i += 8)
    {
        unsigned int t2 = 1 << 7;
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) >= n) break;
            if (compressedData[i + j]) temp |= t2;
            t2 >>= 1;
        }
        out.push_back(static_cast<uint8_t>(temp));
        temp = 0;
    }

    return out;
}

vector<uint8_t> decompressHuffman2Mem(const vector<uint8_t> &compressed)
{
    if (compressed.size() < sizeof(long long) + 1 + 8)
        throw runtime_error("huffman2 compressed data truncated");

    size_t readPos = 0;
    long long totalSymbols = 0;
    memcpy(&totalSymbols, compressed.data() + readPos, sizeof(totalSymbols));
    readPos += sizeof(totalSymbols);

    if (totalSymbols == 0)
        return {};

    unsigned char totalCompressedCl = compressed[readPos++];

    string CLStr;
    for (int i = 0; i < 8; i++)
    {
        if (readPos >= compressed.size()) throw runtime_error("unexpected end of CL header");
        unsigned char CL = compressed[readPos++];
        CLStr += intToBinaryStr(CL, 8);
    }

    vector<unsigned char> codeLengths;
    int j = 0;
    for (int i = 0; i < 19; i++)
    {
        string temp;
        for (int k = 0; k < 3; k++) temp.push_back(CLStr[j++]);
        codeLengths.push_back(binaryToNumber(temp));
    }

    HuffmanCode2 codeLengthDecompressor(codeLengths);
    map<string, unsigned char> decodingTable = codeLengthDecompressor.getDecodings();

    vector<unsigned char> compressedCL;
    vector<unsigned char> extraBitsCL;
    string currCode;
    bool isExtraBits   = false;
    int  extraBitCount = 0;

    while ((compressedCL.size() < totalCompressedCl || isExtraBits) && readPos < compressed.size())
    {
        uint8_t dataByte = compressed[readPos++];
        int bit = 1 << 7;
        for (int i = 0; i < 8; i++)
        {
            if (!isExtraBits && compressedCL.size() >= totalCompressedCl) break;
            currCode.push_back((dataByte & bit) ? '1' : '0');
            if (isExtraBits)
            {
                extraBitCount--;
                if (!extraBitCount)
                {
                    isExtraBits = false;
                    extraBitsCL.push_back(binaryToNumber(currCode));
                    currCode = "";
                }
            }
            else
            {
                if (decodingTable.count(currCode))
                {
                    unsigned char cl = decodingTable[currCode];
                    compressedCL.push_back(cl);
                    currCode = "";
                    if (cl >= 16)
                    {
                        isExtraBits   = true;
                        extraBitCount = (cl == 16) ? 2 : (cl == 17) ? 3 : 7;
                    }
                }
            }
            bit >>= 1;
        }
    }

    vector<unsigned char> ogCodeLengths;
    decompressCodeLengths(compressedCL, extraBitsCL, ogCodeLengths);
    decodingTable = HuffmanCode2(ogCodeLengths).getDecodings();

    string output;
    currCode = "";
    while ((long long)output.size() < totalSymbols && readPos < compressed.size())
    {
        uint8_t byte = compressed[readPos++];
        int bit = 1 << 7;
        for (int i = 0; i < 8 && (long long)output.size() < totalSymbols; i++)
        {
            currCode.push_back((byte & bit) ? '1' : '0');
            if (decodingTable.count(currCode))
            {
                output.push_back(decodingTable[currCode]);
                currCode = "";
            }
            bit >>= 1;
        }
    }

    return vector<uint8_t>(output.begin(), output.end());
}

void compressHuffman2(const string &inFile, const string &outFile)
{
    string input = readFile(inFile);
    vector<uint8_t> inBytes(input.begin(), input.end());
    try
    {
        vector<uint8_t> comp = compressHuffman2Mem(inBytes);
        ofstream file(outFile, ios::binary);
        if (!file) { cerr << "Error: Could not open file " << outFile << endl; return; }
        file.write(reinterpret_cast<const char *>(comp.data()), comp.size());
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }
}

void decompressHuffman2(const string &inFile, const string &outFile)
{
    string input = readFile(inFile);
    vector<uint8_t> inBytes(input.begin(), input.end());
    try
    {
        vector<uint8_t> decomp = decompressHuffman2Mem(inBytes);
        ofstream ofile(outFile, ios::binary);
        if (!ofile) { cerr << "Error: Could not open file " << outFile << endl; return; }
        ofile.write(reinterpret_cast<const char *>(decomp.data()), decomp.size());
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }
}

