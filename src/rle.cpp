#include "../include/rle.hpp"

string rleCompress(const string &input)
{
    char   prev  = input[0];
    int    count = 1;
    int    n     = (int)input.length();
    string output;

    for (int i = 1; i < n; i++)
    {
        if (input[i] == prev)
            count++;
        else
        {
            if (count > 1) output += to_string(count);
            output += prev;
            count = 1;
        }
        prev = input[i];
    }
    if (count > 1) output += to_string(count);
    output += prev;
    return output;
}

string rleDecompress(const string &input)
{
    int    digit  = 1;
    string output;
    int    n = (int)input.length();

    for (int i = 0; i < n; i++)
    {
        int x = input[i];
        if (x >= '0' && x <= '9')
        {
            string digitStr;
            int    j;
            for (j = i; j < n; j++)
            {
                int c = input[j];
                if (!(c >= '0' && c <= '9')) break;
                digitStr += (char)c;
            }
            i     = j - 1;
            digit = stoi(digitStr);
        }
        else
        {
            while (digit--) output += (char)x;
            digit = 1;
        }
    }
    return output;
}
