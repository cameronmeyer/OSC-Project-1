#include <stack>
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

////////////////////
/* MEMORY SECTION */
////////////////////

const string WHITESPACE = "\n\r\t\f\v";
const int start_address = 0;
const int start_system_code = 1000;
const int memory_size = 2000;
int memory[memory_size];

int PC, SP, IR, AC, X, Y;
int timer = 0;
int tempReg = 0;
int cpu_to_mem[2];
int mem_to_cpu[2];
bool kernelState = false;

int read_from_mem(int address)
{
    if(address > -1 && (address < start_system_code || (kernelState && address < memory_size)))
    {
        write(cpu_to_mem[1], &address, sizeof(address));
        int result;
        read(mem_to_cpu[0], &result, sizeof(result));
        return result;
    }
    else
    {
        string error_message = "Invalid memory access at address: " to_string(address);
        throw invalid_argument(error_message);
        exit(-1); 
    }
}

bool write_to_mem(int address, int data)
{
    if(address > -1 && (address < start_system_code || (kernelState && address < memory_size)))
    {
        int write_flag = -1;
        write(cpu_to_mem[1], &write_flag, sizeof(write_flag));
        write(cpu_to_mem[1], &address, sizeof(address));
        write(cpu_to_mem[1], &data, sizeof(data));
    }
    else
    {
        string error_message = "Invalid memory write at address: " to_string(address);
        throw invalid_argument(error_message);
        exit(-1);
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
            cout << "Reading from " << flag << " a value of: " << value << endl;
            write(mem_to_cpu[1], &value, sizeof(value));
        }
        else
        {
            read(cpu_to_mem[0], &address, sizeof(address));
            read(cpu_to_mem[0], &value, sizeof(value));
            memory[address] = value;
            cout << "Writing to " << address << " a value of: " << value << endl;
        }
    }
}

////////////////
/* CPU SECTION*/
////////////////


void execute()
{
    int address = 0;
    int tempReg = 0;
    PC = 0;
    SP = start_system_code - 1;

    while(true)
    {
        IR = read_from_mem(PC);
        // check for interrupt
        // read from memory into IR
        switch(IR)
        {
            case 1:     // Load Value
                // Load the value into the AC
                PC++;
                AC = read_from_mem(PC);
                break;
            case 2:     // Load Address
                // Load the value at the address into the AC
                PC++;
                AC = read_from_mem(read_from_mem(PC)); //read the value from the address specified by PC
                break;
            case 3:     // LoadInd Address
                // Load the value from the address found in the given address into the AC
                break;
            case 4:     // LoadIdxX Address
                // Load the value at (address+X) into the AC
                break;
            case 5:     // LoadIdxY Address
                // Load the value at (address+Y) into the AC
                break;
            case 6:     // LoadSpX
                // Load from (Sp+X) into the AC 
                AC = read_from_mem(SP + X);
                break;
            case 7:     // Store Address
                // Store the value in the AC into the address
                PC++;
                tempReg = read_from_mem(PC);
                write_to_mem(tempReg, AC);
                break;
            case 8:     // Get
                // Gets a random int from 1 to 100 into the AC
                break;
            case 9:     // Put Port
                // If port=1, writes AC as an int to the screen
                // If port=2, writes AC as a char to the screen
                PC++;
                tempReg = read_from_mem(PC);
                //should we be printing a \n for a char?
                if(tempReg == 1)
                {
                    printf("%i\n", AC); //print as int
                }
                else if(tempReg == 2)
                {
                    printf("%c\n", AC); //print as char
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
                PC = read_from_mem(PC) - 1; // Decrement now bc it will be incremented at the end of the loop
                break;
            case 21:    // JumpIfEqual Address
                // Jump to the address only if the value in the AC is zero
                PC++;
                if(AC == 0)
                {
                    PC = read_from_mem(PC) - 1;
                }
                break;
            case 22:    // JumpIfNotEqual Address
                // Jump to the address only if the value in the AC is not zero
                PC++;
                if(AC != 0)
                {
                    PC = read_from_mem(PC) - 1;
                }
                break;
            case 23:    // Call Address
                // Push return address onto stack, jump to the address
                SP--;
                write_to_mem(SP, PC);
                break;
            case 24:    // Ret
                // Pop return address from the stack, jump to the address
                PC = read_from_mem(SP);
                SP++;
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

                // TODO: TEST THIS LATER
                SP--;
                write_to_mem(SP, AC);
                break;
            case 28:    // Pop
                // Pop from stack into AC

                // TODO: TEST THIS LATER!
                AC = read_from_mem(SP);
                SP++;
                break;
            case 29:    // Int
                // Perform system call
                if(kernelState != 0)
                {
                    break;
                }
                kernelState = 1;


                break;
            case 30:    // IRet
                // Return from system call
                break;
            case 50:    // End
                // End execution
                /*for(int i = 0; i < memory_size; i++)
                {
                    cout << i << " " << memory[i] << endl;
                }*/
                return;
                //break;
            //default:    // Invalid Operation
                // throw some error, not a valid operation
        }
        timer++;
        PC++;
    }
}

int main(int argc, char *argv[])
{
    if (argc > 0)
    {
        // call memory execute function
        loadMemory(argv[1]);
    }
    int cpu_mem_success = pipe(cpu_to_mem);
    int mem_cpu_success = pipe(mem_to_cpu);
    if(cpu_mem_success == -1 || mem_cpu_success == -1)
    {
        return -1;
    }

    pid_t pid;

    switch (pid = fork())
    {
        case -1:
            printf("The fork failed!");
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


