#include <bits/stdc++.h>
#include "Priority Queue/arrayImplementation.cpp"
#include <cassert>
using namespace std;

string intToBinaryStr(int num,int minBits = -1){
    if(num ==0) {
        if(minBits == -1) return "0";
        string output;
        while(output.length()<minBits) output.insert(output.begin(),'0');
        return output;

    }
    string output;
    int temp = 1;
    while(temp <= num) temp <<= 1; 
    temp >>= 1;
    while(temp) {output.push_back((temp & num )== 0 ? '0':'1'); temp >>=1;}
    if(minBits !=-1){
        while(output.length()<minBits) output.insert(output.begin(),'0');
    }
    return output;
}
unsigned char BinaryToNumber(string binary){
    int n = binary.size();
    if(n>8) {
        cout << "out of range binary input\n";
        return 0;
    }
    unsigned num = 0;
    for(auto x:binary){
        num <<=1;
        if(x=='1') {
            num |= 1;
        }
    }
    return num;
};
class HuffmanCode
{
    Node *root = nullptr;
    int itemSize = 256;
    PriorityQueue queue;
    int totalInputLen = 0;
    string InputContent;
    map<unsigned char, string> encodings;
    map<string, unsigned char> decodings;
    vector<unsigned char> codeLengths ;

    void buildCodingLengthTree(Node *root, unsigned char codeLen)
    {
        if(!root) return;
        if (!root->left && !root->right){
            codeLengths[root->data] = codeLen;
            return;
        }
        if(root->left)  buildCodingLengthTree(root->left, codeLen + 1 );
        if(root->right) buildCodingLengthTree(root->right, codeLen + 1);
    }
    void buildDecodingTable()
    {
        if (decodings.size())
            return;
        for (auto x : encodings)
        {
            decodings[x.second] = x.first;
        }
    }

  
    void buildFrequencyTable(string input)
    {
        InputContent = input;
        totalInputLen = input.length();
        for (auto x : input)
        {
            frequencies[x]++;
        }
    }
    void buildFrequencyTable(vector<unsigned char> input)
    {
        totalInputLen = input.size();
        for (auto x : input)
        {
            frequencies[x]++;
        }
    }
    void init()
    {

        for (auto x : frequencies)
        {
            queue.enqueue(new Node(x.second, x.first));
        }

        int size = queue.getSize();
        
        while (size >= 1)
        {
            Node *left = queue.dequeue();
            Node *right = queue.dequeue();
            
            if (!left && !right)
            break;
            
            int rf = right ? right->frequency : 0;
            int lf = left ? left->frequency : 0;
            Node *subRoot = new Node(rf+lf);
            subRoot->left = left;
            subRoot->right = right;
            queue.enqueue(subRoot);
            if(size == 1) break;
            size = queue.getSize();
        }
        root = queue.dequeue();
        buildCodingLengthTree(root,0);
        
        buildEncodingTable();
        buildDecodingTable();
    }
    void buildEncodingTable(){
        
        vector<pair<unsigned char, unsigned char>> codeLenthsMap{};
        for (int i = 0; i < this->itemSize; i++)
        {
            if (codeLengths[i])
                codeLenthsMap.push_back({(unsigned char)i, codeLengths[i]});
        }
        sort(codeLenthsMap.begin(), codeLenthsMap.end(), [](auto &a, auto &b)
             {
            if(a.second == b.second)return a.first < b.first;
            return a.second < b.second; });

        int code = 0;
        string codeString;
        int j = 0;

        for (int i = 0; i < codeLenthsMap.size(); i++)
        {
            codeString = "";
            if(i!=0)code += 1;
            int temp = code;
            while (j<codeLenthsMap[i].second){
                code <<= 1;
                j++;
            }
            for (int k = j-1; k >= 0; k--)
            {
                if(code & (1<<k)) codeString.push_back('1');
                else codeString.push_back('0');
            }
            encodings[codeLenthsMap[i].first] = codeString;
        }
    }
    
public:
    map<unsigned char, int> frequencies;

    HuffmanCode(string input)
    {
        this->codeLengths = vector<unsigned char>(this->itemSize, 0);
        buildFrequencyTable(input);
        init();
        
    }
    HuffmanCode(vector<unsigned char> input,int size)
    {
        this->itemSize = size;
        this->codeLengths = vector<unsigned char>(this->itemSize, 0);
        buildFrequencyTable(input);
        init();
    }

    HuffmanCode(vector<unsigned char> codeLengths){
        this->codeLengths = codeLengths;
        this->itemSize = codeLengths.size();
        buildEncodingTable();
        buildDecodingTable();
    }
    
    vector<unsigned char> getCodeLengths()const {return this->codeLengths;}

    map<unsigned char, string> getEncodings()
    {
        return encodings;
    }
    map<string, unsigned char> getDecodings()
    {
        return decodings;
    }
 
   
    vector<bool> encode()
    {
        vector<bool> compressed;
        string output;
        for (auto x : InputContent)
        {
            output += encodings[x];
        }
        for (auto x : output)
        {
            compressed.push_back(x - '0');
        }

        return compressed;
    }
    string decode(vector<bool> &encodedStr)
    {
        string output;
        string temp;
        for (auto x : encodedStr)
        {
            temp.push_back(x ? '1' : '0');
            if (decodings.count(temp))
            {
                output.push_back(decodings[temp]);
                temp = "";
            }
        }
        return output;
    }
};



string readFile(const string &filepath)
{

    ifstream file(filepath, ios::binary);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filepath << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

vector<bool> encodeCL(vector<unsigned char>&compressedCodeLengths,map<unsigned char, string> &encodings,vector<string> &extraBits)
{
    vector<bool> compressed;
    int j =0 ;
    for (auto x : compressedCodeLengths){
        for (auto x : encodings[x])
        {
            compressed.push_back(x - '0');
        }
        if (x > 15){
            for (auto x : extraBits[j++]){
                compressed.push_back(x-'0');
            }
        }
    }

    return compressed;
}

void comrpessCodeLengths(const vector<unsigned char> &codeLengths, vector<unsigned char> &compressed, vector<string> &extraBits)
{
    int n = codeLengths.size();
    if (!n)
        return;
    unsigned char last = codeLengths[0];

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
                extraBits.push_back(intToBinaryStr(min(127, streak - 11),7));
                streak -= min(138, streak);
            }

            if (streak < 3)
            {
                while (streak--)
                    compressed.push_back(codeLengths[i - 1]);
            }
            else
            {

                if (codeLengths[i - 1] == 0)
                {

                    compressed.push_back((unsigned char)17);
                    extraBits.push_back(intToBinaryStr(streak - 3,3));
                }
                else
                {
                    compressed.push_back(codeLengths[i - 1]);
                    streak--;
                    while (streak >= 3)
                    {
                        compressed.push_back((unsigned char)16);
                        extraBits.push_back(intToBinaryStr(min(3, streak - 3),2));
                        streak -= min(6, streak);
                    }

                    while (streak--)
                        compressed.push_back(codeLengths[i - 1]);
                }
            }
            streak = 1;
        }
    }
}

void decompressCodeLenghts(const vector<unsigned char> &compressed, const vector<unsigned char> &extraBits, vector<unsigned char> &codeLengths)
{
    int n = compressed.size();
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        if (compressed[i] > 15)
        {
            if (compressed[i] == 16)
            {
                int count = extraBits[j++] + 3;
                unsigned char lastPushed = codeLengths[codeLengths.size() - 1];
                while (count--)
                {
                    codeLengths.push_back(lastPushed);
                }
            }
            if (compressed[i] == 17)
            {
                int count = extraBits[j++] + 3;
                while (count--)
                {
                    codeLengths.push_back(0);
                }
            }
            if (compressed[i] == 18)
            {
                int count = extraBits[j++] + 11;
                while (count--)
                {
                    codeLengths.push_back(0);
                }
            }
        }
        else
        {
            codeLengths.push_back(compressed[i]);
        }
    }
}

void compressHuffman(string inFile, string outFile)
{
    string input = readFile(inFile);
    HuffmanCode compressor(input);
    vector<unsigned char> codeLengths = compressor.getCodeLengths();
    vector<bool> compressedData = compressor.encode();

    vector<unsigned char> compressedCL ;
    vector<string> extraBitsCL;
    comrpessCodeLengths(codeLengths,compressedCL,extraBitsCL);

    HuffmanCode compressorCL(compressedCL,19);
    map<unsigned char,string> encodings = compressorCL.getEncodings(); 
    vector<bool> compressedCodeLengths = encodeCL(compressedCL,encodings,extraBitsCL);
    


    long long totalChars = input.length();
    unsigned char totalCompressedCL = compressedCL.size();

    ofstream file(outFile, ios::binary);
    if (!file)
    {
        cerr << "Error: Could not open file " << outFile << endl;
        return;
    }
    file.write(reinterpret_cast<const char *>(&totalChars), sizeof(totalChars));
    file.write(reinterpret_cast<const char *>(&totalCompressedCL), sizeof(totalCompressedCL));

    for (auto x : compressorCL.getCodeLengths()){
        file.write(reinterpret_cast<const char *>(&x), sizeof(x));
    }
    
    char temp=0;
    int n = compressedCodeLengths.size();
    for (int i = 0; i < n; i += 8){
        unsigned int t2 = 1 << 7;
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) >= n)
                break;
            if (compressedCodeLengths[i + j])
                temp |= t2;
            t2 >>= 1;
        }
        file.write(reinterpret_cast<const char *>(&temp), sizeof(temp));
        temp = 0;
    }

    temp = 0;
    n = compressedData.size();
    for (int i = 0; i < n; i += 8)
    {
        unsigned int t2 = 1 << 7;
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) >= n)
                break;
            if (compressedData[i + j])
                temp |= t2;
            t2 >>= 1;
        }
        file.write(reinterpret_cast<const char *>(&temp), sizeof(temp));
        temp = 0;
    }

    file.close();
  
     
};

void decompressHuffman(string inFile, string outFile){
    ifstream file(inFile, ios::binary);
    if (!file)
    {
        cerr << "Error: Could not open file " << inFile << endl;
        return;
    }
    
    long long totalSymbols = 0;
    unsigned char totalCompressedCl= 0;
    file.read(reinterpret_cast<char *>(&totalSymbols), sizeof(totalSymbols));
    file.read(reinterpret_cast<char *>(&totalCompressedCl), sizeof(totalCompressedCl));

    vector<unsigned char> codeLengths{}; // uses the same for CL and main codeLengths for data

    for (int i = 0; i < 19; i++)
    {
        unsigned char codeLen;
        file.read(reinterpret_cast< char *>(&codeLen), sizeof(codeLen));
        codeLengths.push_back(codeLen); 
    }
    HuffmanCode codeLengthDecompressor(codeLengths);
    map<string, unsigned char> decodingTable = codeLengthDecompressor.getDecodings();

    vector<unsigned char> compressedCL;
    vector<unsigned char> extraBitsCL;
    char dataByte;
    string currCode = "";
    bool isExtraBits = 0;
    int extraBitCount = 0;
  
    // to load the compressed CL and extra bit CL arrays
    while ((compressedCL.size() < totalCompressedCl || isExtraBits) && file.read(&dataByte, 1))
    {
        int bit = 1 << 7;
        for (int i = 0; i < 8 ; i++)
        {
            if (!isExtraBits && compressedCL.size() >= totalCompressedCl)
            {
                break;
            }
            currCode.push_back(dataByte & bit ? '1' : '0');
            if(isExtraBits){
                extraBitCount--;
                if(!extraBitCount){
                    isExtraBits = false;
                    extraBitsCL.push_back(BinaryToNumber(currCode));
                    currCode = "";
                }
            }
            else{
                
                if (decodingTable.count(currCode))
                {
                    unsigned char cl = decodingTable[currCode];
                    compressedCL.push_back(cl);
                    currCode = "";

                    if (cl >= 16)
                    {
                        isExtraBits = true;
                        if (cl == 16)
                            extraBitCount = 2;
                        else if (cl == 17)
                            extraBitCount = 3;
                        else if (cl == 18)
                            extraBitCount = 7;
                    }
                }
            }
                bit >>= 1;
            }
        }
 
    vector<unsigned char> ogCodeLengths{};
    decompressCodeLenghts(compressedCL,extraBitsCL,ogCodeLengths);
    decodingTable = HuffmanCode(ogCodeLengths).getDecodings();
    string output;
    char byte;
    currCode = "";
    while ((long long)output.size() < totalSymbols && file.read(&byte, sizeof(byte)))
    {
        int bit = 1 << 7;
        for (int i = 0; i < 8 && (long long)output.size() < totalSymbols; i++)
        {
            currCode.push_back(byte & bit ? '1' : '0');
            if (decodingTable.count(currCode))
            {
                output.push_back(decodingTable[currCode]);
                currCode = "";
            }
            bit >>= 1;
        }
    }
    file.close();

    ofstream ofile(outFile, ios::binary);
    if (!ofile)
    {
        cerr << "Error: Could not open file " << outFile << endl;
        return;
    }
    ofile << output;
    ofile.close();
    cout << "file has been decompressed : " << outFile << "\n";
    return;
}



int main()
{
    compressHuffman("huffman.cpp", "data/compressed.bin");
    decompressHuffman("data/compressed.bin", "data/huffman.cpp");

    // string input = readFile("data/input.txt");
    // HuffmanCode hf(input);
    // vector<unsigned char> compressed;
    // vector<string> extraBits;
    // vector<unsigned char> ogCodeLengths = hf.getCodeLengths();
    // comrpessCodeLengths(ogCodeLengths,compressed,extraBits);
    // HuffmanCode hf2(compressed,19);

    // vector<unsigned char> recoveredCodeLengths ;
    // expandCodeLenghts(compressed, extraBits, recoveredCodeLengths);

  
    
    return 0;
}