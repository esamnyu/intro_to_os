#include "oslabs.h"
#include <limits.h>

#define QUEUEMAX 50

const struct PCB NULLPCB = {0, 0, 0, 0, 0, 0, 0};

// Function to handle the arrival of a process for execution
struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp);

// Function to handle the arrival of a process for execution
// This function handles the arrival of a new process in a Priority-based Preemptive Scheduler.
// It updates the ready queue and returns the process to execute next.
struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    if (is_same_PCB(current_process, NULLPCB)) {
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
    }
    // Return the PCB of the process to execute next
}

// Function to handle the completion of a process execution
// This function handles the completion of a process execution in a Priority-based Preemptive Scheduler.
// It updates the ready queue and returns the completed process.
struct PCB handle_process_completion_pp(struct PCB ready_queue[], int *queue_count, int *execution_starttime, int timestamp) {
    struct PCB completed_process;
    completed_process = ready_queue[0];
    completed_process.execution_endtime = timestamp;
    
    // Shift the remaining processes in the ready queue
    for (int i = 0; i < *queue_count - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_count)--;
    
    // Update the execution start time of the next process (if any)
    if (*queue_count > 0) {
        *execution_starttime = timestamp;
    }
    
    // Return the completed process
    return completed_process;
}

// Function to implement Priority-based Preemptive Scheduling
// This function selects the process to execute next based on the priority in a Priority-based Preemptive Scheduler.
struct PCB priority_based_preemptive_scheduling(struct PCB ready_queue[], int queue_count, int timestamp) {
    struct PCB selected_process = NULLPCB;
    int highest_priority = INT_MAX;
    
    // Find the process with the highest priority that is ready to execute
    for (int i = 0; i < queue_count; i++) {
        if (ready_queue[i].execution_endtime <= timestamp) {
            continue;
        }
        if (ready_queue[i].process_priority < highest_priority) {
            highest_priority = ready_queue[i].process_priority;
            selected_process = ready_queue[i];
        }
    }
    
    // Return the PCB of the selected process
    return selected_process;
}

// Function to implement Shortest-Remaining-Time-Next Preemptive Scheduling
// This function selects the process to execute next based on the shortest remaining time in a Shortest-Remaining-Time-Next Preemptive Scheduler.
struct PCB shortest_remaining_time_next(struct PCB ready_queue[], int queue_count, int timestamp) {
    struct PCB selected_process = NULLPCB;
    int shortest_remaining_time = INT_MAX;
    
    // Find the process with the shortest remaining time that is ready to execute
    for (int i = 0; i < queue_count; i++) {
        if (ready_queue[i].execution_endtime <= timestamp) {
            continue
