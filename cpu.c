#include "oslabs.h"
#include <limits.h>

const struct PCB NULLPCB = {0, 0, 0, 0, 0, 0, 0};

struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    if (current_process.process_id == 0) { // Assuming process_id = 0 means it's the NULLPCB
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        new_process.remaining_bursttime = new_process.total_bursttime;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
        return new_process;
    }
    else {
        new_process.remaining_bursttime = new_process.total_bursttime;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
        if (new_process.process_priority < current_process.process_priority) {
            return new_process;
        } else {
            return current_process;
        }
    }
}

struct PCB handle_process_completion_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    if (*queue_cnt == 0) {
        return NULLPCB;  // Return NULLPCB if the ready queue is empty
    }

    int next_process_index = 0;
    for (int i = 0; i < *queue_cnt; i++) {
        if (ready_queue[i].remaining_bursttime < ready_queue[next_process_index].remaining_bursttime) {
            next_process_index = i;
        }
    }

    struct PCB next_process = ready_queue[next_process_index];

    // Shift the remaining processes in the ready queue
    for (int i = next_process_index; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_cnt)--;

    next_process.execution_starttime = timestamp;
    next_process.execution_endtime = timestamp + next_process.remaining_bursttime;

    return next_process;
}


struct PCB handle_process_completion_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    struct PCB completed_process;
    completed_process = ready_queue[0];
    
    // Shift the remaining processes in the ready queue
    for (int i = 0; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_cnt)--;
    
    // Select the next process based on the highest priority
    struct PCB next_process = NULLPCB;
    int highest_priority = INT_MAX;
    for (int i = 0; i < *queue_cnt; i++) {
        if (ready_queue[i].process_priority < highest_priority) {
            highest_priority = ready_queue[i].process_priority;
            next_process = ready_queue[i];
        }
    }

    // Update the execution start time of the next process (if any)
    if (next_process.process_id != 0) {
        next_process.execution_starttime = timestamp;
        next_process.execution_endtime = timestamp + next_process.remaining_bursttime;
    }

    return next_process;
}

struct PCB handle_process_arrival_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    new_process.remaining_bursttime = new_process.total_bursttime;

    if (current_process.process_id == 0) { // Assuming process_id = 0 means it's the NULLPCB
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
        return new_process;
    }
    else {
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
        if (new_process.remaining_bursttime < current_process.remaining_bursttime) {
            return new_process;
        } else {
            return current_process;
        }
    }
}
