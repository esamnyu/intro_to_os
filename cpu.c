#include "oslabs.h"
#include <limits.h>

#define QUEUEMAX 50

const struct PCB NULLPCB = {0, 0, 0, 0, 0, 0, 0};

struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp);

struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    if (is_same_PCB(current_process, NULLPCB)) {
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
    }
    // Added the closing brace here
}

struct PCB handle_process_completion_pp(struct PCB ready_queue[], int *queue_count, int *execution_starttime, int timestamp) {
    struct PCB completed_process;
    completed_process = ready_queue[0];
    completed_process.execution_endtime = timestamp;
    
    for (int i = 0; i < *queue_count - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_count)--;
    
    if (*queue_count > 0) {
        *execution_starttime = timestamp;
    }
    
    return completed_process;
}

struct PCB priority_based_preemptive_scheduling(struct PCB ready_queue[], int queue_count, int timestamp) {
    struct PCB selected_process = NULLPCB;
    int highest_priority = INT_MAX;
    
    // Find the process with the highest priority
    for (int i = 0; i < queue_count; i++) {
        if (ready_queue[i].execution_endtime <= timestamp) {
            continue;
        }
        if (ready_queue[i].process_priority < highest_priority) {
            highest_priority = ready_queue[i].process_priority;
            selected_process = ready_queue[i];
        }
    }
    
    return selected_process;
}

struct PCB shortest_remaining_time_next(struct PCB ready_queue[], int queue_count, int timestamp) {
    struct PCB selected_process = NULLPCB;
    int shortest_remaining_time = INT_MAX;
    
    // Find the process with the shortest remaining time
    for (int i = 0; i < queue_count; i++) {
        if (ready_queue[i].execution_endtime <= timestamp) {
            continue;
        }
        if (ready_queue[i].remaining_bursttime < shortest_remaining_time) {
            shortest_remaining_time = ready_queue[i].remaining_bursttime;
            selected_process = ready_queue[i];
        }
    }
    
    return selected_process;
}

struct PCB round_robin(struct PCB ready_queue[], int queue_count, int *execution_starttime, int timestamp, int time_quantum) {
    struct PCB selected_process = NULLPCB;
    
    if (selected_process.process_id != 0) {
        *execution_starttime = timestamp;
    }
    
    // Check if the current process has completed its time quantum
    if (timestamp - *execution_starttime >= time_quantum) {
        selected_process = ready_queue[0];
    }
    
    return selected_process;
}
