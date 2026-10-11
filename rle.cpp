#include <bits/stdc++.h>
using namespace std;

string compress(string input){
    char prev = input[0];
    int count = 1;
    int n = input.length();
    string output;
    for (int i = 1; i < n; i++)
    {
        if (input[i] == prev)
            count++;
        else{
            if(count>1) 
                output += to_string(count);
            output += prev;
            count = 1;
        }
        prev = input[i];
    }
    if (count > 1)
        output += to_string(count);
    output += prev;
    return output;
}

string decompress(string input){
    int digit = 1;
    int factor = 1;
    string output;
    int n = input.length();
    for(int i=0;i<n;i++){
        int x = input[i];
        if(x >= '0' && x<='9'){
            string digitStr = "";
            int j;
            for (j = i; j < n ; j++){
                int x = input[j];
                if (! (x >= '0' && x <= '9') ) break;
                digitStr+=x;
            }
                i = j-1;
                digit = stoi(digitStr);
            }
            else{
                while(digit--) output += x;
                digit =1; 
            }
            
    }
    return output;
}

int main() {
    string input;
    cin >> input;


    string compressedStr =  compress(input);
    cout << compressedStr << "\n";
    cout << decompress(compressedStr);
    cout << float((float)compressedStr.length()/input.length()) << "\n";


    return 0;
}