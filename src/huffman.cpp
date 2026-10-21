#include "../include/huffman.hpp"
#include "../include/utils.hpp"

// ── Private helpers ───────────────────────────────────────────────────────────

void HuffmanCode::buildEncodingTable(Node *root, const string &code)
{
    if (!root->left && !root->right) { encodings[root->data] = code; return; }
    buildEncodingTable(root->left,  code + "0");
    buildEncodingTable(root->right, code + "1");
}

void HuffmanCode::buildDecodingTable()
{
    if (decodings.size()) return;
    for (auto x : encodings)
        decodings[x.second] = x.first;
}

float HuffmanCode::getEntorpy(Node *root)
{
    if (!root) return 0.0f;
    return (float)root->frequency / totalInputLen * (float)encodings[root->data].length()
         + getEntorpy(root->left) + getEntorpy(root->right);
}

void HuffmanCode::buildFrequencyTable(const string &input)
{
    InputContent  = input;
    totalInputLen = (int)input.length();
    for (auto x : input) frequencies[x]++;
}

void HuffmanCode::init()
{
    for (auto x : frequencies)
        queue.enqueue(new Node(x.second, x.first));

    int size = queue.getSize();
    while (size > 1)
    {
        Node *left  = queue.dequeue();
        Node *right = queue.dequeue();
        if (!left || !right) break;
        Node *subRoot  = new Node(right->frequency + left->frequency);
        subRoot->left  = left;
        subRoot->right = right;
        queue.enqueue(subRoot);
        size = queue.getSize();
    }
    root = queue.dequeue();
    buildEncodingTable(root, "");
    buildDecodingTable();
}

void HuffmanCode::init(map<unsigned char, int> freq)
{
    this->frequencies = freq;
    init();
}

// ── Constructors ──────────────────────────────────────────────────────────────

HuffmanCode::HuffmanCode(const string &input)
{
    buildFrequencyTable(input);
    init();
}

HuffmanCode::HuffmanCode(map<unsigned char, int> freq)
{
    init(freq);
}

// ── Public accessors ──────────────────────────────────────────────────────────

map<unsigned char, string> HuffmanCode::getEncodings() { return encodings; }
map<string, unsigned char> HuffmanCode::getDecodings() { return decodings; }
float                      HuffmanCode::getEntropy()   { return getEntorpy(root); }

vector<bool> HuffmanCode::originalBin()
{
    vector<bool> binary;
    for (auto x : InputContent)
    {
        char j = 1 << 7;
        for (int i = 0; i < 8; i++) { binary.push_back(x & j); j >>= 1; }
    }
    for (auto x : binary) cout << x;
    return binary;
}

vector<bool> HuffmanCode::encode()
{
    vector<bool> compressed;
    string       output;
    for (auto x : InputContent) output += encodings[x];
    for (auto x : output)       compressed.push_back(x - '0');
    return compressed;
}

string HuffmanCode::decode(vector<bool> &encodedStr)
{
    string output, temp;
    for (auto x : encodedStr)
    {
        temp.push_back(x ? '1' : '0');
        if (decodings.count(temp)) { output.push_back(decodings[temp]); temp = ""; }
    }
    return output;
}

// ── File-level compress ───────────────────────────────────────────────────────

void compressHuffman(const string &filePath, const string &outFile)
{
    string                  input      = readFile(filePath);
    HuffmanCode             compressor(input);
    map<unsigned char, int> frequencies = compressor.frequencies;
    vector<bool>            compressed  = compressor.encode();

    char padding    = (char)((8 - compressed.size() % 8) % 8);
    int  totalChars = (int)frequencies.size();

    ofstream file(outFile, ios::binary);
    if (!file) { cerr << "Error: Could not open file " << outFile << endl; return; }

    file.write(reinterpret_cast<const char *>(&padding),    sizeof(padding));
    file.write(reinterpret_cast<const char *>(&totalChars), sizeof(totalChars));
    for (auto x : frequencies)
    {
        file.write(reinterpret_cast<const char *>(&x.first),  sizeof(x.first));
        file.write(reinterpret_cast<const char *>(&x.second), sizeof(x.second));
    }

    char temp = 0;
    int  n    = (int)compressed.size();
    for (int i = 0; i < n; i += 8)
    {
        unsigned int t2 = 1 << 7;
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) >= n) break;
            if (compressed[i + j]) temp |= t2;
            t2 >>= 1;
        }
        file.write(reinterpret_cast<const char *>(&temp), sizeof(temp));
        temp = 0;
    }
    file.close();
}

// ── File-level decompress ─────────────────────────────────────────────────────

void decompressHuffman(const string &inputFile, const string &outFile)
{
    ifstream file(inputFile, ios::binary);
    if (!file) { cerr << "Error: Could not open file " << inputFile << endl; return; }

    char padding    = 0;
    int  totalChars = 0;
    file.read(&padding, sizeof(padding));
    file.read(reinterpret_cast<char *>(&totalChars), sizeof(totalChars));

    map<unsigned char, int> frequencies;
    long long               totalSymbols = 0;
    for (int i = 0; i < totalChars; i++)
    {
        char token;
        int  frequency;
        file.read(&token, sizeof(token));
        file.read(reinterpret_cast<char *>(&frequency), sizeof(frequency));
        frequencies[(unsigned char)token] = frequency;
        totalSymbols += frequency;
    }

    HuffmanCode            hf(frequencies);
    map<string, unsigned char> decodingTable = hf.getDecodings();
    string output;
    char   byte;
    string currCode;

    while ((long long)output.size() < totalSymbols && file.read(&byte, sizeof(byte)))
    {
        int bit = 1 << 7;
        for (int i = 0; i < 8 && (long long)output.size() < totalSymbols; i++)
        {
            currCode.push_back(byte & bit ? '1' : '0');
            if (decodingTable.count(currCode)) { output.push_back(decodingTable[currCode]); currCode = ""; }
            bit >>= 1;
        }
    }
    file.close();

    ofstream ofile(outFile, ios::binary);
    if (!ofile) { cerr << "Error: Could not open file " << outFile << endl; return; }
    ofile << output;
    ofile.close();
}
