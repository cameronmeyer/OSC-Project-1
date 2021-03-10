Cameron Meyer
cdm180003
Project 1
CS 4348.001


-- COMMANDS TO COMPILE AND RUN THE PROGRAM --
g++ -std=c++11 main.cpp -o a.out
./a.out <input filename> <timer interrupt count>


-- LIST OF FILES --
main.cpp
   Contains code to simulate a computer system's CPU and memory.
   Can accept input from a text file and interpret contents as
   instructions to be executed. 

sample1.txt
   [PROVIDED EXAMPLE FILE]
   Tests the indexed load instructions.
   Prints two tables, one of A-Z, the other of 1-10.

sample2.txt
   [PROVIDED EXAMPLE FILE]
   Tests the call/ret instructions.
   Prints a face where the lines are printed using subroutine calls.

sample3.txt
   [PROVIDED EXAMPLE FILE]
   Tests the int/iret instructions.
   The main loop is printing the letter A followed by a number
   that is being periodically incremented by the timer.
   The number will increment faster if the timer period is
   shorter.

sample4.txt
   [PROVIDED EXAMPLE FILE]
   Tests the proper operation of the user stack and system
   stack, and also tests that accessing system memory in 
   user mode gives an error and exits.

sample5.txt
   Tests the call/ret instructions.
   Prints "UTD" in bold ASCII letters. Sections of the art
   are broken into subroutine calls for more efficient printing. 

summary.pdf
   A document describing the project purpose, how the project was
   implemented, and Cameron's personal experience in doing the project.