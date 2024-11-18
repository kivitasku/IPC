# Inter-Process Communication

A simple project to demonstrate and test inter-process communication via pipes and shared memory.

## Basics

- scheduler.cpp: Starts the program and reads values from shared memory.
- init.cpp: Generates random numbers and passes them to shared memory.
- init.h: Header file for accessing init from scheduler.
- compile.sh: A quick shell script for compiling.

## Notice
- Project uses unix system calls, thus it works only on linux distros.
