#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <random>
#include <climits>
#include <ctime>

using namespace std;

int main() {
    const int ARRAY_SIZE = 20;
    int arr[ARRAY_SIZE];
    int pipefd[2]; // File descriptors for pipe
    pid_t pid;
    
    // Initialize random number generator
    srand(time(nullptr));
    
    // Fill array with random numbers (1-100 for better readability)
    cout << "Array elements: ";
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 100 + 1;
        cout << arr[i] << " ";
    }
    cout << endl << endl;
    
    // Create pipe before fork
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    
    // Create child process
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    else if (pid == 0) {
        // CHILD PROCESS - find minimum in second half (elements 10-19)
        close(pipefd[0]); // Close read end of pipe
        
        int child_min = INT_MAX;
        cout << "Child Process (PID: " << getpid() << ")" << endl;
        cout << "Searching second half of array (indices 10-19): ";
        
        for (int i = 10; i < ARRAY_SIZE; i++) {
            cout << arr[i] << " ";
            if (arr[i] < child_min) {
                child_min = arr[i];
            }
        }
        
        cout << endl;
        cout << "Child found minimum: " << child_min << endl << endl;
        
        // Write minimum to pipe
        write(pipefd[1], &child_min, sizeof(child_min));
        close(pipefd[1]); // Close write end of pipe
        
        return 0; // Child process exits
    }
    else {
        // PARENT PROCESS - find minimum in first half (elements 0-9)
        close(pipefd[1]); // Close write end of pipe
        
        int parent_min = INT_MAX;
        int child_min;
        
        cout << "Parent Process (PID: " << getpid() << ")" << endl;
        cout << "Searching first half of array (indices 0-9): ";
        
        for (int i = 0; i < 10; i++) {
            cout << arr[i] << " ";
            if (arr[i] < parent_min) {
                parent_min = arr[i];
            }
        }
        
        cout << endl;
        cout << "Parent found minimum: " << parent_min << endl << endl;
        
        // Wait for child to complete and read from pipe
        wait(nullptr); // Wait for child process to finish
        read(pipefd[0], &child_min, sizeof(child_min));
        close(pipefd[0]); // Close read end of pipe
        
        // Determine overall minimum
        int overall_min = (parent_min < child_min) ? parent_min : child_min;
        
        // Display results
        cout << "=== RESULTS ===" << endl;
        cout << "Parent Process (PID: " << getpid() << ") minimum: " << parent_min << endl;
        cout << "Child Process minimum (received via pipe): " << child_min << endl;
        cout << "Overall minimum of entire array: " << overall_min << endl;
    }
    
    return 0;
}
