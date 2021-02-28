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

int read(int address);
bool write(int address, int data);
void loadMemory(string fileName);

int PC, SP, IR, AC, X, Y;

void execute()
{
    int address = 0;
    while(true)
    {
        // check for interrupt
        // read from memory into IR
        switch(IR)
        {
            case 1:     // Load Value
                // Load the value into the AC
                break;
            case 2:     // Load Address
                // Load the value at the address into the AC
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
                AC = read(SP + X);
                break;
            case 7:     // Store Address
                // Store the value in the AC into the address
                break;
            case 8:     // Get
                // Gets a random int from 1 to 100 into the AC
                break;
            case 9:     // Put Port
                // If port=1, writes AC as an int to the screen
                // If port=2, writes AC as a char to the screen
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
                break;
            case 21:    // JumpIfEqual Address
                // Jump to the address only if the value in the AC is zero
                break;
            case 22:    // JumpIfNotEqual Address
                // Jump to the address only if the value in the AC is not zero
                break;
            case 23:    // Call Address
                // Push return address onto stack, jump to the address
                break;
            case 24:    // Ret
                // Pop return address from the stack, jump to the address
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
                break;
            case 28:    // Pop
                // Pop from stack into AC
                break;
            case 29:    // Int
                // Perform system call
                break;
            case 30:    // IRet
                // Return from system call
                break;
            case 50:    // End
                // End execution
                return;
                //break;
            //default:    // Invalid Operation
                // throw some error, not a valid operation
        }
    }
}

int main(int argc, char *argv[])
{
    pid_t pid;

    switch (pid = fork())
    {
    case -1:
        /* Here pid is -1, the fork failed */
        /* Some possible reasons are that you're */
        /* out of process slots or virtual memory */
        printf("The fork failed!");
        exit(-1);

    case 0:
        /* pid of zero is the child */
        printf("Child:  hello!\n");
        if (argc > 1)
        {
            cout << argv[1];
            // call memory execute function
            loadMemory(argv[1]);
        }
        _exit(0);

    default:
        /* pid greater than zero is parent */

        execute();

        printf("Parent: child's pid is %d\n", pid);
    }

    return 0;
}


