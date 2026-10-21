#include "../include/utils.hpp"

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
