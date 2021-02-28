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

string trim(const string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == string::npos) ? "" : s.substr(0, end + 1);
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
                if (trimmed.substr(0, 1) == ".")
                {
                    //NOTE: memory address
                   /* int newAddress = readIntegerFromString(trimmed,0);
                    if(instruction > -1)
                    {
                        memory[memory_Address] = instruction;
                    }
                    else
                    {
                        //throw an error
                    }*/                                                  
                }
                //memory_Address++;
            }
        }
        //REMOVE THIS LATER!!!
        cout << endl;
        for(int i - 0; i < memory_size; i)))
        {
            cout << i << " " << memory[i] << endl;
        }
        file.close();
    }
    else
    {
        cout << "File not found." << endl;
    }
}