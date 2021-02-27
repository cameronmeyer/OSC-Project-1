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
string trim(const string& s);

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
        _exit(0);

    default:
        /* pid greater than zero is parent */

        /* wait for child process to end */
        waitpid(-1, NULL, 0);

        printf("Parent: child's pid is %d\n", pid);
    }

    if (argc > 1)
    {
        cout << argv[1];
        loadMemory(argv[1]);
    }

    return 0;
}


