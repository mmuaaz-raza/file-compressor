#include "../include/lzss.hpp"
#include "../include/utils.hpp"

// ── Constants ─────────────────────────────────────────────────────────────────
static const int MAX_LEN    = 255;
static const int MAX_WINDOW = 256;

// ── compressLZSSAbs ───────────────────────────────────────────────────────────
void compressLZSSAbs(const string &input,
                     vector<bool>          &flags,
                     vector<unsigned char> &data)
{
    int i = 0;
    int n = (int)input.length();
    while (i < n)
    {
        int sti     = max(0, i - MAX_WINDOW);
        int bestLen = 0, bestOffset = 0;

        for (int j = sti; j < i; j++)
        {
            int len = 0;
            while (i + len < n && j + len < n && input[i + len] == input[j + len])
            {
                len++;
                if (len >= MAX_LEN + 3) break;
            }
            if (bestLen < len)
            {
                bestLen    = len;
                bestOffset = i - j;
            }
            if (len == MAX_LEN + 3) break;
        }

        if (bestLen < 3)
        {
            flags.push_back(false);
            data.push_back((unsigned char)input[i]);
            i++;
        }
        else
        {
            flags.push_back(true);
            data.push_back((unsigned char)(bestLen - 3));
            data.push_back((unsigned char)(bestOffset - 1));
            i += bestLen;
        }
    }
}

// ── decompressLZSSAbs ─────────────────────────────────────────────────────────
string decompressLZSSAbs(vector<bool>          &flags,
                         vector<unsigned char> &data)
{
    int    i = 0;
    string output;
    for (bool x : flags)
    {
        if (x)
        {
            unsigned char offset = data[i++] + 1;
            unsigned char length = data[i++] + 3;
            int start_pos = (int)output.size() - offset;
            for (int j = 0; j < length; j++)
                output.push_back(output[start_pos + j]);
        }
        else
        {
            output.push_back((char)data[i++]);
        }
    }
    return output;
}

vector<uint8_t> compressLZSSMem(const vector<uint8_t> &input)
{
    string fileContent(reinterpret_cast<const char *>(input.data()), input.size());
    uint64_t totalChars = input.size();

    vector<uint8_t> out;
    const uint8_t *tcPtr = reinterpret_cast<const uint8_t *>(&totalChars);
    out.insert(out.end(), tcPtr, tcPtr + sizeof(totalChars));

    if (totalChars == 0) return out;

    vector<bool>          flags;
    vector<unsigned char> data;
    compressLZSSAbs(fileContent, flags, data);

    unsigned char temp    = 0;
    int           i       = 7;
    int           dataPtr = 0;

    for (bool x : flags)
    {
        if (x)
        {
            temp |= (1 << (i--));
            if (i < 0) { out.push_back(temp); temp = 0; i = 7; }

            unsigned char length = data[dataPtr++];
            char lengthPtr = 7;
            while (lengthPtr >= 0)
            {
                bool bitVal = (length & (1 << (lengthPtr--)));
                temp |= bitVal << (i--);
                if (i < 0) { out.push_back(temp); temp = 0; i = 7; }
            }

            unsigned char offset = data[dataPtr++];
            char offsetPtr = 7;
            while (offsetPtr >= 0)
            {
                bool bitVal = (offset & (1 << (offsetPtr--)));
                temp |= bitVal << (i--);
                if (i < 0) { out.push_back(temp); temp = 0; i = 7; }
            }
        }
        else
        {
            i--;
            if (i < 0) { out.push_back(temp); temp = 0; i = 7; }

            char dataPoint = (char)data[dataPtr++];
            char dataPointPtr = 7;
            while (dataPointPtr >= 0)
            {
                bool bitVal = (dataPoint & (1 << (dataPointPtr--)));
                temp |= bitVal << (i--);
                if (i < 0) { out.push_back(temp); temp = 0; i = 7; }
            }
        }
    }
    if (i != 7) out.push_back(temp);
    return out;
}

vector<uint8_t> decompressLZSSMem(const vector<uint8_t> &compressed)
{
    if (compressed.size() < sizeof(uint64_t))
        return {};

    uint64_t totalChars = 0;
    memcpy(&totalChars, compressed.data(), sizeof(totalChars));
    if (totalChars == 0) return {};

    size_t byteIdx = sizeof(uint64_t);
    int bitIdx = 7;

    auto readBit = [&]() -> int {
        if (byteIdx >= compressed.size()) return -1;
        int bit = (compressed[byteIdx] >> bitIdx) & 1;
        bitIdx--;
        if (bitIdx < 0) { bitIdx = 7; byteIdx++; }
        return bit;
    };

    string output;
    output.reserve(totalChars);

    while (output.size() < totalChars)
    {
        int flag = readBit();
        if (flag == -1) break;

        if (flag == 1)
        {
            unsigned char length = 0;
            for (int lp = 7; lp >= 0; lp--)
            {
                int b = readBit();
                if (b == -1) goto done;
                length |= (b << lp);
            }

            unsigned char offset = 0;
            for (int op = 7; op >= 0; op--)
            {
                int b = readBit();
                if (b == -1) goto done;
                offset |= (b << op);
            }

            int actualOffset = static_cast<int>(offset) + 1;
            int actualLength = static_cast<int>(length) + 3;

            if (static_cast<size_t>(actualOffset) > output.size())
                break;

            size_t start = output.size() - actualOffset;
            for (int k = 0; k < actualLength && output.size() < totalChars; k++)
            {
                output.push_back(output[start + k]);
            }
        }
        else
        {
            char data = 0;
            for (int dp = 7; dp >= 0; dp--)
            {
                int b = readBit();
                if (b == -1) goto done;
                data |= (b << dp);
            }
            output.push_back(data);
        }
    }

done:
    return vector<uint8_t>(output.begin(), output.end());
}

void compressLZSS(const string &inFile, const string &outFile)
{
    string fileContent = readFile(inFile);
    vector<uint8_t> inBytes(fileContent.begin(), fileContent.end());
    vector<uint8_t> comp = compressLZSSMem(inBytes);

    ofstream file(outFile, ios::binary);
    if (!file) { cerr << "Error: Could not open file " << outFile << endl; return; }
    file.write(reinterpret_cast<const char *>(comp.data()), comp.size());
}

void decompressLZSSBin(const string &inFile, const string &outFile)
{
    string fileContent = readFile(inFile);
    vector<uint8_t> inBytes(fileContent.begin(), fileContent.end());
    vector<uint8_t> decomp = decompressLZSSMem(inBytes);

    ofstream oFile(outFile, ios::binary);
    if (!oFile) { cerr << "Error: Could not open file " << outFile << endl; return; }
    oFile.write(reinterpret_cast<const char *>(decomp.data()), decomp.size());
}

