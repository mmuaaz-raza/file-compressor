#include "LZSS.cpp"
using namespace std;


int main(){
    string HuffmancompressedFile = "compressedHuffman.bin"; 
    string LZSScompressedFile = "compressedLZSS.txt";
    compressLZSS("data/input.txt","data/"+LZSScompressedFile);
    compressHuffman("data/"+LZSScompressedFile,"data/"+HuffmancompressedFile);
    decompressHuffman("data/" + HuffmancompressedFile, "data/" + LZSScompressedFile);
    decompressLZSSBin("data/" + LZSScompressedFile,"data/og.txt");
}