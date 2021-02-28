/*#include <stack>
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

int read_from_mem(int address)
{
    if(address > -1 && address < memory_size)
    {
        return memory[address];
    }
    else
    {
        // throw an out of bounds error
    }
}

bool write_to_mem(int address, int data)
{
    if(address > -1 && address < memory_size)
    {
        memory[address] = data;
        return true;
    }
    else
    {
        // throw an out of bounds error
        return false;
    }
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

void executeMemory()
{
    int flag; //memory index
    int address; 
    int value;
    string str;

    while(true)
    {
        //read from cpu->mem pipe and store value in flag
        read(cpu_to_mem[0], &flag, sizeof(flag));
        if(flag > -1)
        {
            value = memory[flag];
        }
    }
}*/