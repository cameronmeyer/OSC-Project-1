#include <stack>
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <iostream>
using namespace std;

const string WHITESPACE = "\n\r\t\f\v";
const int start_address = 0;
const int start_system_code = 1000;
const int memory_size = 2000;

int read(int address)
{
    return 0;
}

bool write(int address, int data)
{
    return 0;
}

void loadMemory(string fileName)
{
    ifstream file;
    cout << fileName;
    file.open(fileName);
    if (file)
    {
        string line;
        while (getline(file, line))
        {
            string trimmed = trim(line);
            if (trimmed != "")
            {
                cout << trimmed << endl;
            }
        }
        file.close();
    }
    else
    {
        cout << "File not found." << endl;
    }
}

string trim(const string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == string::npos) ? "" : s.substr(0, end + 1);
}