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

const string WHITESPACE = "\n\r\t\f\v"; // Whitespace characters for parsing the input file
const int startSystemCode = 1000;       // Starting index of system code
const int timerIndex = 1000;            // Index used for timer interrupts
const int syscallIndex = 1500;          // Index used for system calls
const int memoryCapacity = 2000;        // Total indices used for memory
int memory[memoryCapacity];             // Memory array

int PC, SP, IR, AC, X, Y;               // Registers
int timer = 0;                          // Timer
int timerIterations = 0;                // Number of iterations necessary to trigger a timer interrupt
int temp = 0;                           // Temporary register to help with processing
int cpuToMem[2];                        // File descriptor for CPU -> memory pipe
int memToCpu[2];                        // File descriptor for memory -> CPU pipe
bool kernelState = false;               // Toggle for entering the kernel state

////////////////////
/* MEMORY SECTION */
////////////////////

int readFromMem(int address)
{
    // Ensure address is within proper bounds depending on if it is in the kernel state
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
    // Ensure address is within proper bounds depending on if it is in the kernel state
    if(address > -1 && (address < startSystemCode || (kernelState && address < memoryCapacity)))
    {
        int writeFlag = -1;
        write(cpuToMem[1], &writeFlag, sizeof(writeFlag));
        write(cpuToMem[1], &address, sizeof(address));
        write(cpuToMem[1], &data, sizeof(data));
        return true;
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
    string number = s.substr(0, s.find(WHITESPACE)); // Only use the beginning of the line until whitespace is found
    try
    {
        // Convert string to int
        std::string::size_type sz;
        int result = stoi(number, &sz);
        return vector<int>{1, result}; // Index 0 in this vector indicates a success
    }
    catch(...)
    {
        return vector<int>{-1, 0}; // Index 0 in this vector indicates a failure for int conversion
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

        // Continuously read each line of input from the file
        while(getline(file, input))
        {
            // Check if the line is a memory address
            if(input.substr(0, 1) == ".")
            {
                int tempAddress = getIntFromString(input.substr(1, input.length()))[1] - 1; // Decrement by 1 since memory address will be added to later
                if(tempAddress >= -1 && tempAddress < memoryCapacity) // Ensure the address is within bounds of the memory
                {
                    memoryAddress = tempAddress;
                }
                else
                {
                    string errorMessage = "Error: Invalid memory write at address: " + to_string(tempAddress);
                    throw out_of_range(errorMessage);
                    exit(-1);
                }
            }
            else
            {
                vector<int> readInt = getIntFromString(input);
                int instruction = readInt[1];

                // If we successfully read an int from the input line
                if(readInt[0] != -1)
                {
                    if(memoryAddress >= 0 && memoryAddress < memoryCapacity) // Ensure the address is within bounds of the memory
                    {
                        memory[memoryAddress] = instruction;
                    }
                    else
                    {
                        string errorMessage = "Error: Invalid memory write at address: " + to_string(memoryAddress);
                        throw out_of_range(errorMessage);
                        exit(-1);
                    }
                }
                else
                {
                    memoryAddress--; // Skip an entire line of input (used for blank lines, comments, etc.)
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
    int address;        // If writing to memory, this is the address to write to
    int data;           // Value being written to memory or read from memory
    int readWriteFlag;  // -1 if writing data to memory, contains an address if reading from memory

    while(true)
    {
        // Read from CPU -> memory pipe to check if we will read or write our memory
        read(cpuToMem[0], &readWriteFlag, sizeof(readWriteFlag));

        if(readWriteFlag > -1) // If the flag is greater than -1 it will contain an address we can read from
        {
            data = memory[readWriteFlag];
            write(memToCpu[1], &data, sizeof(data));
        }
        else // Write data into the appropriate memory address
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

void kernelMode() // Triggers the kernel state and pushes SP and PC to the stack
{
    kernelState = true;
    temp = SP;
    SP = memoryCapacity;
    pushStack(temp);
    pushStack(PC);
}

void execute()
{
    int address = 0;
    PC = 0;
    SP = startSystemCode;

    while(true)
    {
        // Check for a timer interrupt if we aren't already in the kernel state
        if(timer % timerIterations == 0 && timerIterations > 0 && timer > 0 && !kernelState)
        {
            PC--;
            kernelMode();
            PC = timerIndex;
        }

        // Fetch the current command from memory
        IR = readFromMem(PC);

        switch(IR)
        {
            case 1:     // Load Value
                PC++;
                AC = readFromMem(PC);
                break;
            case 2:     // Load Address
                PC++;
                AC = readFromMem(readFromMem(PC));
                break;
            case 3:     // LoadInd Address
                PC++;
                AC = readFromMem(readFromMem(readFromMem(PC)));
                break;
            case 4:     // LoadIdxX Address
                PC++;
                AC = readFromMem(readFromMem(PC) + X);
                break;
            case 5:     // LoadIdxY Address
                PC++;
                AC = readFromMem(readFromMem(PC) + Y);
                break;
            case 6:     // LoadSpX
                AC = readFromMem(SP + X);
                break;
            case 7:     // Store Address
                PC++;
                temp = readFromMem(PC);
                writeToMem(temp, AC);
                break;
            case 8:     // Get random int (1 to 100) and store in AC
                srand(floor(time(0) * M_PI) + PC + AC + IR);
                AC = (rand() % 100) + 1;
                break;
            case 9:     // Put Port
                PC++;
                temp = readFromMem(PC);
                if(temp == 1)
                {
                    printf("%i", AC); // Print AC as int
                }
                else if(temp == 2)
                {
                    printf("%c", AC); // Print AC as char
                }
                break;
            case 10:    // AddX
                AC += X;
                break;
            case 11:    // AddY
                AC += Y;
                break;
            case 12:    // SubX
                AC -= X;
                break;
            case 13:    // SubY
                AC -= Y;
                break;
            case 14:    // CopyToX
                X = AC;
                break;
            case 15:    // CopyFromX
                AC = X;
                break;
            case 16:    // CopyToY
                Y = AC;
                break;
            case 17:    // CopyFromY
                AC = Y;
                break;
            case 18:    // CopyToSp
                SP = AC;
                break;
            case 19:    // CopyFromSp
                AC = SP;
                break;
            case 20:    // Jump Address
                PC++;
                PC = readFromMem(PC) - 1; // Decrement now since it will be incremented at the end of the loop to the acutal desired value
                break;
            case 21:    // JumpIfEqual Address
                PC++;
                if(AC == 0)
                {
                    PC = readFromMem(PC) - 1;
                }
                break;
            case 22:    // JumpIfNotEqual Address
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
                PC = popStack();
                break;
            case 25:    // IncX
                X++;
                break;
            case 26:    // DecX
                X--;
                break;
            case 27:    // Push
                pushStack(AC);
                break;
            case 28:    // Pop
                AC = popStack();
                break;
            case 29:    // Int
                if(kernelState)
                {
                    break; // Do not perform a syscall if already in the kernel state
                }
                kernelMode();
                PC = syscallIndex - 1; // Set PC to 1 less than desired since the loop will increment it later
                break;
            case 30:    // IRet
                if(!kernelState)
                {
                    break; // Do not return from the kernel state if you weren't in the kernel state
                }

                PC = popStack();
                SP = popStack();
                kernelState = false;
                break;
            case 50:    // End
                return;
            default:    // Invalid Operation
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
        // Populate memory with contents from input file
        populateMemory(argv[1]);

        // Set iterations until timer interrupt
        if(argc > 1)
        {
            string::size_type sz;
            timerIterations = stoi(argv[2], &sz);
        }
    }

    // Set up pipes
    int validCtoM = pipe(cpuToMem);
    int validMtoC = pipe(memToCpu);
    if(validCtoM == -1 || validMtoC == -1)
    {
        return -1;
    }

    pid_t pid;

    // Fork the process and execute memory or CPU functions
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