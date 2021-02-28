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
int memory[memory_size];

int read(int address)
{
    return 0;
}

bool write(int address, int data)
{
    return 0;
}

vector<int> readIntFromString(string s)
{
    string number = s.substr(0, s.find(WHITESPACE));
    try
    {
        std::string::size_type sz;
        int result = stoi(number, &sz);
        return vector<int>{1, result};
    }
    catch(...)
    {
        return vector<int>{-1, 0};
        cout << ("Not a valid command input");
    }
}

void loadMemory(string fileName)
{
    ifstream file;
    cout << fileName;
    file.open(fileName);
    if (file)
    {
        string line = "";
        int memory_Address = 0;

        while (getline(file, line))
        {
            if(line.substr(0, 1) == ".")
            {
                memory_Address = readIntFromString(line.substr(1, line.length()))[1] - 1; // Decrement by 1 since it'll be added to later
            }
            else
            {
                vector<int> readInt = readIntFromString(line);
                int instruction = readInt[1];

                if(readInt[0] != -1)
                {
                    if(memory_Address >= 0 && memory_Address < memory_size)
                    {
                        memory[memory_Address] = instruction;
                    }
                    else
                    {
                        // throw an out of bounds error
                    }
                }
            }

            memory_Address++;
        }
        //REMOVE THIS LATER!!!
        cout << endl;
        for(int i = 0; i < memory_size; i++)
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