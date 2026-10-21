#include <bits/stdc++.h>
#include "huffman.cpp"
using namespace std;

const int MAX_LEN = 255;
const int MAX_WINDOW = 255;


string decompress(vector<bool> &flags, vector<char> &data);

void compress(string input, vector<bool> &flags, vector<char> &data)
{

    int i= 0;
    int n = input.length();
    while (i<n){
        int sti = max(0,i-MAX_WINDOW);

        unsigned char bestLen= 0;
        unsigned char bestOffset =0;

        for (int j = sti; j < i; j++)
        {
            int len = 0;
            
            while (i+len < n && j+len < n && input[i+len] == input[j+len]){
                len++;
                if(len > MAX_LEN) break;
            }

            
            if(bestLen < len){
                bestLen = len;
                bestOffset = i-j;
            }
        }
        
        if(bestLen< 3){
            flags.push_back(0);
            data.push_back(input[i]);
            i++;
        }
        else{
            flags.push_back(1);
            data.push_back(bestOffset);
            data.push_back(bestLen);
            i+= bestLen;
        }

        
    }

   decompress(flags,data);
}



string decompress(vector<bool> &flags,vector<char> &data){
    int i=0;

    
    string output;
    for (auto x:flags){
        if(x){
            unsigned char offset = data[i++];
            unsigned char length = data[i++];
            int start_pos = output.size() - offset;
            for (int j = 0; j < length; j++){
                output.push_back(output[start_pos+j]);
            }
        }
        else{
            cout << data[i] << "";
            output.push_back(data[i++]);
        }
    }

    return output;
}

void compressLZSS(const string inFile,const string outFile ){
    string fileContent = readFile(inFile);
    vector<bool> flags;
    vector<char> data;

    
    compress(fileContent,flags,data);
    
    
    unsigned char temp = 0 ;
    int i = 7;
    int dataPtr = 0;
    ofstream file(outFile,ios::binary);
    for(auto x:flags){
        if(x){
            temp |= (1<<(i--)); // flag bit has been placed
            if (i < 0)
            {
                file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
                temp = 0;
                i = 7;
            }
            unsigned char offset = data[dataPtr++];
            char offsetPtr = 7;
            while (offsetPtr >=0){
                bool bitVal = (offset & (1<<(offsetPtr--))); 
                temp |=  bitVal << (i--);
                if(i<0){
                    file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
                    temp = 0 ;
                    i = 7;
                }
            }
            unsigned char length = data[dataPtr++];
            char lengthPtr = 7;
            while (lengthPtr >=0)
            {
                bool bitVal = (length & (1 << (lengthPtr--)));
                temp |= bitVal << (i--);
                if (i < 0)
                {
                    file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
                    temp = 0;
                    i = 7;
                }
            }
        }
        else{
            i--; // no need to check the flag, it is 0 
            if(i<0){
                file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
                temp =0;
                i = 7;
            }
            char dataPoint = data[dataPtr++];
            char dataPointPtr = 7;
            while (dataPointPtr >= 0)
            {
                bool bitVal =(dataPoint & (1 << (dataPointPtr--)));
                temp |=  bitVal << (i--);
                if (i < 0)
                {
                    file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
                    temp = 0;
                    i = 7;
                }
            }
        }

    }
    if (i != 7)
        file.write(reinterpret_cast<char *>(&temp), sizeof(temp));
    file.close();
}

void decompressLZSSBin(const string inFile,const string outFile ){
    ifstream file(inFile,ios::binary);
    if(!file){
        cerr << "File couldn't be found";
        return;
    }
    
    unsigned char temp;
    int i= 7 ;
    file.read(reinterpret_cast<char *> (& temp), sizeof(temp));
    string output;
    while (true){
        bool flag = temp & (1<<(i--));
        if (i < 0)
        {
            if (!file.read(reinterpret_cast<char *>(&temp), sizeof(temp)))
                goto end_loop;
            i = 7;
        }
        if(flag){
            unsigned char offset = 0;
            unsigned char length = 0;
            int offsetPtr = 7;
            while (offsetPtr >= 0)
            {
                bool bitVal = temp & (1 << (i--));
                offset |= bitVal << (offsetPtr--);
                if (i < 0)
                {
                    if (!file.read(reinterpret_cast<char *>(&temp), sizeof(temp)))
                        goto end_loop;
                    i = 7;
                }
            }
            int lengthPtr = 7;
            while (lengthPtr >= 0)
            {
                bool bitVal = temp & (1 << (i--));
                length |= bitVal << (lengthPtr--);
                if (i < 0)
                {
                    if (!file.read(reinterpret_cast<char *>(&temp), sizeof(temp)))
                       if(lengthPtr >=0 ) goto end_loop;

                    i = 7;
                }
            }
            int st = output.size() - offset;
            for (int i = st; i < st+length; i++) output.push_back(output[i]);
            
            

        }
        else{
            char data= 0;
            int dataPtr = 7;
            bool end =false;
            while (dataPtr >=0){
                bool bitVal = temp & (1<<(i--)); 
                data |= bitVal << (dataPtr--);
                if(i<0){

                    if (!file.read(reinterpret_cast<char *>(&temp), sizeof(temp)))
                        if(dataPtr >= 0) goto end_loop;
                    i=7;
                }
            }
          
            output.push_back(data);
        }
    }
    end_loop:
        file.close();

    ofstream oFile(outFile,ios::binary);
    if (!oFile)
    {
        cerr << "Error: Could not open file " << outFile << endl;
        return;
    }
    oFile << output;
    cout << "File saved successfully!\n";
    oFile.close();
    return;
    

}

int main() {
    compressLZSS("data/input.txt","data/compressed.bin");
    decompressLZSSBin("data/compressed.bin","data/og.txt");
    return 0;
}