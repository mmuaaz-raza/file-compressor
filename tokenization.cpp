#include <bits/stdc++.h>
#include <cassert>
using namespace std;


struct LenCode
{
   int symbol;
   int extraBits;
   int extraValue;
};
const int lengthBase[29]  = {3,4,5,6,7,8,9,10, 11,13,15,17, 19,23,27,31, 35,43,51,59, 67,83,99,115, 131,163,195,227, 258};
const int lengthExtra[29] = {0,0,0,0,0,0,0,0,  1,1,1,1,     2,2,2,2,     3,3,3,3,     4,4,4,4,       5,5,5,5,       0};
LenCode encodeLength(int length){
    assert(length < 3 || length >258);
    auto i = upper_bound(lengthBase,lengthBase+29,length);
    int index = (i-1) - lengthBase;
    int symbol = 256+index+1;
    int extraLen = length-lengthBase[index];
    return {symbol,lengthExtra[index],extraLen};
}
int decodeLength(LenCode code){
    assert(code.symbol>258);
    return lengthBase[code.symbol-257]+code.extraValue;
}
const int distBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                          257, 385, 513, 769, 1025, 1537, 2049, 3073,
                          4097, 6145, 8193, 12289, 16385, 24577};
const int distExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                           7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
LenCode encodeDistance(int offset){
    
    assert(!(offset <= 0 || offset > 32768) );
    auto i = upper_bound(distBase, distBase + 30, offset);
    int index = (i - 1) - distBase;
    int symbol = index ;
    int extraLen = offset - distBase[index];
    return {symbol, distExtra[index], extraLen};
}
int decodeDistance(LenCode code){
    assert(code.symbol < 30);
    return distBase[code.symbol]+code.extraValue;
}

int main() {
    for (int i = 1; i <= 32768; i++)
    {

        int decodedVal = decodeDistance(encodeDistance(i));
        assert(decodedVal==i);
        
        
    }
    

    return 0;
}