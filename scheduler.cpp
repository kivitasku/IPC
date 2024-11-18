#include <iostream>
#include <algorithm>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "init.h"


//Sorting algorithm from standard library
void sortArray(int array[], int len) {
    std::sort(array, array + len, std::greater<int>());
}

int main() {  
    const int memSize = 2048; //Size of the shared memory 
    const key_t key = 314159; //Hardcoded key: pi

    //Criteria 2:(a, create memory)
    //Create shared memory with IPC_CREAT to create it with permissions to write (0666)
    int shmid = shmget(key, memSize, IPC_CREAT | 0666);
    if (shmid == -1) {
        std::cout << "shmget creating failed" << std::endl;
        return 1;
    }
    std::cout << "Shared memory created" << std::endl;

    //Print PID
    pid_t pidSch = getpid();
    std::cout << "Scheduler ID: " << pidSch << std::endl;

    //Criteria 2:(b, own process)
    //Creating a new process
    pid_t pid = fork(); 

    if (pid < 0) {
        std::cout << "Forking failed" << std::endl;
        return 1;
    } else if (pid == 0) {
        //Starting init with a new process
        init();
        return 0;
    } else {
        //Wait for child process to finish to ensure synchronization
        //and to avoid race conditions
        wait(NULL); 
    }


    //Attaching to the shared memory and casting the void pointer to be a int pointer
    int *ptr = static_cast<int*>(shmat(shmid, nullptr, 0));
    if (ptr == (void *)-1) {
        std::cout << "shmat attaching failed" << std::endl;
        return 1;
    }

    //initializing an array of 4 numbers (P1-P4)
    int priorityNumbers[4];
    //Criteria 2:(b, reading from memory)
    //Reading the numbers from the shared memory and writing them to the array
    for (int i = 0; i < 4; ++i) {
        priorityNumbers[i] = ptr[i];
    }
    std::cout << "Read from shared memory" << std::endl;


    //Detaching from the shared memory
    if (shmdt(ptr) == -1) {
        std::cout << "shmdt detaching failed" << std::endl;
        return 1;
    }
    std::cout << "Scheduler detached from shared memory" << std::endl;

    //Deleting shared memory
    if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
        std::cout << "shmctl deletion failed" << std::endl;
        return 1;
    }
    std::cout << "Deleted shared memory" << std::endl;

    //Criteria 2:(b, sort)
    //Sorting the array
    int len = sizeof(priorityNumbers) / sizeof(priorityNumbers[0]);
    sortArray(priorityNumbers, len);

    //Printing out the sorted array
    std::cout << "Sorted array: " << std::endl;
    for(int i = 0; i < len; i++) {
        std::cout << " " << priorityNumbers[i];
    }
    std::cout << std::endl;


    std::cout << "Exiting" << std::endl;
    return 0;
}
