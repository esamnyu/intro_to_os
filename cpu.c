#include "oslabs.h"
#include <limits.h> // Include the <limits.h> header file

struct PCB NULLPCB = {0, 0, 0, 0, 0, 0, 0}; // Declare and initialize the NULLPCB


// Function to handle the arrival of a process for execution
void arrival(struct PCB ready_queue[], int *queue_count, struct PCB process) {
    ready_queue[*queue_count] = process;       // Add the process to the ready queue
    (*queue_count)++;                          // Increment the queue count
}

// Function to handle the completion of a process execution
struct PCB completion(struct PCB ready_queue[], int *queue_count, int *execution_starttime, int timestamp) {
    struct PCB completed_process;
    completed_process = ready_queue[0];         // Get the process that just completed execution
    completed_process.execution_endtime = timestamp;   // Set the execution end time of the completed process
    
    // Shift the remaining processes in the ready queue
    for (int i = 0; i < *queue_count - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_count)--;                          // Decrement the queue count
    
    // Update the execution start time of the next process (if any)
    if (*queue_count > 0) {
        *execution_starttime = timestamp;
    }
    
    return completed_process;                  // Return the completed process
}

// Function to implement Priority-based Preemptive Scheduling
struct PCB priority_based_preemptive_scheduling(struct PCB ready_queue[], int queue_count, int timestamp) {
    struct PCB selected_process = NULLPCB;      // Initialize the selected process as NULLPCB
    int highest_priority = INT_MAX;             // Initialize the highest priority as maximum
    
    // Find the process with the highest priority
    for (int i = 0; i < queue_count; i++) {
        if (ready_queue[i].process_priority < highest_priority) {
            highest_priority = ready_queue[i].process_priority;
            selected_process = ready_queue[i];
        }
    }
    
    return selected_process;                    // Return the selected process
}

// Function to implement Shortest-Remaining-Time-Next Preemptive Scheduling
struct PCB shortest_remaining_time_next(struct PCB ready_queue[], int queue_count, int timestamp) {
    struct PCB selected_process = NULLPCB;      // Initialize the selected process as NULLPCB
    int shortest_remaining_time = INT_MAX;      // Initialize the shortest remaining time as maximum
    
    // Find the process with the shortest remaining time
    for (int i = 0; i < queue_count; i++) {
        if (ready_queue[i].remaining_bursttime < shortest_remaining_time) {
            shortest_remaining_time = ready_queue[i].remaining_bursttime;
            selected_process = ready_queue[i];
        }
    }
    
    return selected_process;                    // Return the selected process
}

// Function to implement Round-Robin Scheduling
struct PCB round_robin(struct PCB ready_queue[], int queue_count, int *execution_starttime, int timestamp, int time_quantum) {
    struct PCB selected_process = NULLPCB;      // Initialize the selected process as NULLPCB
    
    // Check if the current process has completed its time quantum
    if (timestamp - *execution_starttime >= time_quantum) {
        selected_process = ready_queue[0];       // Select the first process in the ready queue
    }
    
    return selected_process;                    // Return the selected process
}
