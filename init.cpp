#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <random>
#include <sys/shm.h>
#include <cstring>

#define MSGSIZE sizeof(4)



int numGen() {
    static std::random_device rd; // Non-deterministic seed
    static std::mt19937 generator(rd()); // Mersenne Twister generator
    std::uniform_int_distribution<int> distribution(0, 19);
    return distribution(generator);
}

int init() {
    const key_t key = 314159;

    //Try to only get shared memory address and dont create if not found 
    int shmid = shmget(key, 0, 0);
    if (shmid == -1) {
        std::cout << "Shared mem not found" << std::endl;
        return 1; 
    }


    //Print PID to ensure process id's aren't the same
    pid_t pid = getpid();
    std::cout << "Init ID: " << pid << std::endl;



    //initializing array for the child process priorities
    int priorityNumbers[4];

    //Creating a pipe for passing random numbers from P1-P4 to init
    int p[2], i; 
    if (pipe(p) < 0) {
        exit(1);
    } 
    std::cout << "Init-children pipe opened" << std::endl;

    const int childProcesses = 4; // P1 - P4

    //Criteria 1:(a)
    //Fork 4 childs
    for (int i = 0; i < childProcesses; ++i) {
        pid_t cPid = fork();

        if (cPid < 0) {
            std::cout << "Forking failed" << std::endl;
            return 1;
        } else if (cPid == 0) {
            //Writing the random number to the pipe
            //Criteria 1:(b), (c)
            int prioNum = numGen();
            std::cout << "Child " << i << " PID: " << getpid() << " num: " << prioNum << std::endl;
            write(p[1], &prioNum, MSGSIZE);
            std::cout << "Child " << i << " wrote to pipe" << std::endl;
            return 0;
        }
        
    }

    //waiting for all child processes to finish
    //each wait(NULL) waits for one child process termination
    for (int i = 0; i < childProcesses; ++i) {
        wait(NULL);
    }
    std::cout << "All child processes finished" << std::endl;

    //Closing the write end of pipe
    close(p[1]);
    std::cout << "Init-children pipe-write closed" << std::endl;
    


    //Criteria 1:(d)
    //reading from the pipe and saving the numbers to the array
    for (int i = 0; i < childProcesses; ++i) {
        int number;
        read(p[0], &number, MSGSIZE);
        priorityNumbers[i] = number;
    }
    std::cout << "Read data from pipe" << std::endl;

    //Printing the numbers before sorting
    std::cout << "Numbers in the array: " << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << priorityNumbers[i] << " ";
    }
    std::cout << std::endl;



    //Closing pipe's read end
    close(p[0]);
    std::cout << "Init-children pipe-read closed" << std::endl;



    //Criteria 3:(a)
    //Attaching to the shared memory
    void *ptr = shmat(shmid, nullptr, 0);
     if (ptr == (void *)-1) {
        std::cout << "shmat failed" << std::endl;
        return 1;
    }
    std::cout << "Init attaching to shared memory" << std::endl; 
    
    //Criteria 3:(b)
    //Writing the array to the shared memory
    memcpy(ptr, priorityNumbers, sizeof(priorityNumbers));
    std::cout << "Wrote to shared memory" << std::endl;

    //Detaching from shared memory
     if (shmdt(ptr) == -1) {
        std::cout << "shmdt failed" << std::endl;
        return 1;
    }
    std::cout << "Init detaching from shared memory" << std::endl;





    //Closing init
    std::cout << "Closing init" << std::endl;
    return 0;
}
