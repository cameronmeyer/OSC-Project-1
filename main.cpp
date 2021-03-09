#include <stack>
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdexcept>
#include <exception>
#include <unistd.h>
#include <cmath>
#include <math.h>
using namespace std;

const string WHITESPACE = "\n\r\t\f\v";
const int startSystemCode = 1000;
const int timerIndex = 1000;
const int syscallIndex = 1500;
const int memoryCapacity = 2000;
int memory[memoryCapacity];

int PC, SP, IR, AC, X, Y;
int timer = 0;
int timerIterations = 0;
int temp = 0;
int cpuToMem[2];
int memToCpu[2];
bool kernelState = false;

////////////////////
/* MEMORY SECTION */
////////////////////

int readFromMem(int address)
{
    if(address > -1 && (address < startSystemCode || (kernelState && address < memoryCapacity)))
    {
        int data;
        write(cpuToMem[1], &address, sizeof(address));
        read(memToCpu[0], &data, sizeof(data));
        return data;
    }
    else
    {
        string errorMessage = "Error: Invalid memory access at address: " + to_string(address);
        throw invalid_argument(errorMessage);
        exit(-1); 
    }
}

bool writeToMem(int address, int data)
{
    if(address > -1 && (address < startSystemCode || (kernelState && address < memoryCapacity)))
    {
        int writeFlag = -1;
        write(cpuToMem[1], &writeFlag, sizeof(writeFlag));
        write(cpuToMem[1], &address, sizeof(address));
        write(cpuToMem[1], &data, sizeof(data));
    }
    else
    {
        string errorMessage = "Error: Invalid memory write at address: " + to_string(address);
        throw invalid_argument(errorMessage);
        exit(-1);
    }
}

vector<int> getIntFromString(string s)
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
        cout << "Not a valid command input" << endl;
    }
}

void populateMemory(string fileName)
{
    ifstream file;
    file.open(fileName);
    if(file)
    {
        string input = "";
        int memoryAddress = 0;

        while(getline(file, input))
        {
            if(input.substr(0, 1) == ".")
            {
                int tempAddress = getIntFromString(input.substr(1, input.length()))[1] - 1;
                if(tempAddress >= -1 && tempAddress < memoryCapacity)
                {
                    memoryAddress = tempAddress; // Decrement by 1 since it'll be added to later
                }
                else
                {
                    // throw an out of bounds error
                    string errorMessage = "Error: Invalid memory write at address: " + to_string(tempAddress)  + " with a PC of " + to_string(PC);
                    throw out_of_range(errorMessage);
                    exit(-1);
                }
            }
            else
            {
                vector<int> readInt = getIntFromString(input);
                int instruction = readInt[1];

                if(readInt[0] != -1)
                {
                    if(memoryAddress >= 0 && memoryAddress < memoryCapacity)
                    {
                        memory[memoryAddress] = instruction;
                    }
                    else
                    {
                        // throw an out of bounds error
                        string errorMessage = "Error: Invalid memory write at address: " + to_string(memoryAddress);
                        throw out_of_range(errorMessage);
                        exit(-1);
                    }
                }
                else
                {
                    memoryAddress--; //If an entire line of input should be skipped (blank lines, comments, etc.)
                }
            }

            memoryAddress++;
        }
        file.close();
    }
    else
    {
        string errorMessage = "Error: File " + fileName + " not found.";
        throw invalid_argument(errorMessage);
        exit(-1);
    }
}

void executeMemory()
{
    int address; // if writing to memory, this is the address to write to
    int data; // value being written to memory or read from memory
    int readWriteFlag; // -1 if writing data to memory, contains an address if reading from memory

    while(true)
    {
        //read from cpu->mem pipe to check if we will read or write our memory
        read(cpuToMem[0], &readWriteFlag, sizeof(readWriteFlag));
        if(readWriteFlag > -1) // if the flag is greater than -1 it will contain an address we can read from
        {
            data = memory[readWriteFlag];
            write(memToCpu[1], &data, sizeof(data));
        }
        else
        {
            read(cpuToMem[0], &address, sizeof(address));
            read(cpuToMem[0], &data, sizeof(data));
            memory[address] = data;
        }
    }
}

/////////////////
/* CPU SECTION */
/////////////////

void pushStack(int data)
{
    SP--;
    writeToMem(SP, data);  
}

int popStack()
{
    int temp = readFromMem(SP);
    SP++;
    return temp;
}

void kernelMode()
{
    kernelState = true;
    temp = SP;
    SP = memoryCapacity;// - 1;
    pushStack(temp);
    pushStack(PC);
}

void execute()
{
    int address = 0;
    PC = 0;
    SP = startSystemCode;// - 1;

    while(true)
    {
        //cout << "TIMER VALUE: " << timer << endl;
        if(timer % timerIterations == 0 && timerIterations > 0 && timer > 0 && !kernelState)
        {
            //cout << "-- ENTERING KERNEL MODE (timer interrupt) -- " << endl;
            PC--;
            kernelMode();
            PC = timerIndex;
        }

        IR = readFromMem(PC);
        // check for interrupt
        // read from memory into IR
        switch(IR)
        {
            case 1:     // Load Value
                // Load the value into the AC
                PC++;
                AC = readFromMem(PC);
                break;
            case 2:     // Load Address
                // Load the value at the address into the AC
                PC++;
                AC = readFromMem(readFromMem(PC)); //read the value from the address specified by PC
                break;
            case 3:     // LoadInd Address
                // Load the value from the address found in the given address into the AC
                PC++;
                AC = readFromMem(readFromMem(readFromMem(PC)));
                break;
            case 4:     // LoadIdxX Address
                // Load the value at (address+X) into the AC
                PC++;
                AC = readFromMem(readFromMem(PC) + X);
                break;
            case 5:     // LoadIdxY Address
                // Load the value at (address+Y) into the AC
                PC++;
                AC = readFromMem(readFromMem(PC) + Y);
                break;
            case 6:     // LoadSpX
                // Load from (Sp+X) into the AC 
                AC = readFromMem(SP + X);
                break;
            case 7:     // Store Address
                // Store the value in the AC into the address
                PC++;
                temp = readFromMem(PC);
                writeToMem(temp, AC);
                break;
            case 8:     // Get
                // Gets a random int from 1 to 100 into the AC
                srand(floor(time(0) * M_PI) + PC + AC + IR);
                AC = (rand() % 100) + 1;
                break;
            case 9:     // Put Port
                // If port=1, writes AC as an int to the screen
                // If port=2, writes AC as a char to the screen
                PC++;
                temp = readFromMem(PC);
                //should we be printing a \n for a char?
                if(temp == 1)
                {
                    printf("%i", AC); //print as int
                }
                else if(temp == 2)
                {
                    printf("%c", AC); //print as char
                }
                break;
            case 10:    // AddX
                // Add the value in X to the AC
                AC += X;
                break;
            case 11:    // AddY
                // Add the value in Y to the AC
                AC += Y;
                break;
            case 12:    // SubX
                // Subtract the value in X from the AC
                AC -= X;
                break;
            case 13:    // SubY
                // Subtract the value in Y from the AC
                AC -= Y;
                break;
            case 14:    // CopyToX
                // Copy the value in the AC to X
                X = AC;
                break;
            case 15:    // CopyFromX
                // Copy the value in X to the AC
                AC = X;
                break;
            case 16:    // CopyToY
                // Copy the value in the AC to Y
                Y = AC;
                break;
            case 17:    // CopyFromY
                // Copy the value in Y to the AC
                AC = Y;
                break;
            case 18:    // CopyToSp
                // Copy the value in AC to the SP
                SP = AC;
                break;
            case 19:    // CopyFromSp
                // Copy the value in SP to the AC 
                AC = SP;
                break;
            case 20:    // Jump Address
                // Jump to the address
                PC++;
                PC = readFromMem(PC) - 1; // Decrement now bc it will be incremented at the end of the loop
                break;
            case 21:    // JumpIfEqual Address
                // Jump to the address only if the value in the AC is zero
                PC++;
                if(AC == 0)
                {
                    PC = readFromMem(PC) - 1;
                }
                break;
            case 22:    // JumpIfNotEqual Address
                // Jump to the address only if the value in the AC is not zero
                PC++;
                if(AC != 0)
                {
                    PC = readFromMem(PC) - 1;
                }
                break;
            case 23:    // Call Address
                // Push return address onto stack, jump to the address
                PC++;
                pushStack(PC);
                PC = readFromMem(PC) - 1; // Decrement now bc it will be incremented at the end of the loop
                break;
            case 24:    // Ret
                // Pop return address from the stack, jump to the address
                PC = popStack();
                break;
            case 25:    // IncX
                // Increment the value in X
                X++;
                break;
            case 26:    // DecX
                // Decrement the value in X
                X--;
                break;
            case 27:    // Push
                // Push AC onto stack
                pushStack(AC);
                break;
            case 28:    // Pop
                // Pop from stack into AC
                AC = popStack();
                break;
            case 29:    // Int
                // Perform system call
                if(kernelState)
                {
                    break;
                }
                //cout << "-- ENTERING KERNEL MODE (syscall) -- " << endl;
                kernelMode();
                PC = syscallIndex - 1; // will be incremented at the end of the loop
                break;
            case 30:    // IRet
                // Return from system call
                if(!kernelState)
                {
                    break;
                }
                //cout << "-- EXITING KERNEL MODE (ac = " << AC << ", sp = " << SP << ") --" << endl;
                PC = popStack();
                SP = popStack();
                //temp = SP + 1;
                //PC = readFromMem(temp);
                //temp++;
                //SP = readFromMem(temp) + 1;
                kernelState = false;
                break;
            case 50:    // End
                // End execution
                return;
            default:    // Invalid Operation
                // throw error, not a valid operation
                string errorMessage = "Error: Invalid CPU instruction of value: " + to_string(IR);
                throw invalid_argument(errorMessage);
                exit(-1);
        }
        timer++;
        PC++;
    }
}

int main(int argc, char *argv[])
{
    if(argc > 0)
    {
        // call memory execute function
        populateMemory(argv[1]);

        if(argc > 1)
        {
            string::size_type sz;
            timerIterations = stoi(argv[2], &sz);
        }
    }
    int validCtoM = pipe(cpuToMem);
    int validMtoC = pipe(memToCpu);
    if(validCtoM == -1 || validMtoC == -1)
    {
        return -1;
    }

    pid_t pid;

    switch(pid = fork())
    {
        case -1:
            printf("Error: The fork failed.");
            return -1;
        case 0:
            executeMemory();
            return 0;
        default:
            execute();
            return 0;
    }
    return 0;
}