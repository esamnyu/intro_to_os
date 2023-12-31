#include "oslabs.h"
#include <limits.h>
#include <stdlib.h>
#include<stdio.h>

const struct PCB NULLPCB = {0, 0, 0, 0, 0, 0, 0};

struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    new_process.remaining_bursttime = new_process.total_bursttime;

    // If there is no currently-running process (i.e., the third argument is the NULLPCB)
    if (current_process.process_id == 0) {
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        // The new process is not added to the ready queue here, but directly returned to run
        return new_process;
    } else {
        // If there is a currently-running process
        if (new_process.process_priority < current_process.process_priority) {
            // If the new process has a higher priority
            current_process.remaining_bursttime -= timestamp - current_process.execution_starttime;
            current_process.execution_endtime = 0; // Adjusting its execution end time as 0 as per instructions
            ready_queue[*queue_cnt] = current_process; 
            (*queue_cnt)++;
            new_process.execution_starttime = timestamp;
            new_process.execution_endtime = timestamp + new_process.total_bursttime;
            // Again, the new process is not added to the ready queue here, but directly returned to run
            return new_process;
        } else {
            // If the new process has equal or lower priority
            new_process.execution_starttime = 0;
            new_process.execution_endtime = 0;
            ready_queue[*queue_cnt] = new_process;
            (*queue_cnt)++;
            return current_process; 
        }
    }
}


struct PCB handle_process_completion_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    // If the ready queue is empty, return NULLPCB
    if (*queue_cnt == 0) {
        return NULLPCB;
    }

    // Initialize minimum priority variable and index of the highest priority process
    int min_priority = INT_MAX;
    int high_priority_index = 0;

    // Find the PCB of the process in the ready queue with the highest priority
    for (int i = 0; i < *queue_cnt; i++) {
        if (ready_queue[i].process_priority < min_priority) {
            min_priority = ready_queue[i].process_priority;
            high_priority_index = i;
        }
    }

    // Save the highest priority PCB
    struct PCB high_priority_pcb = ready_queue[high_priority_index];
    
    // Set the execution start time as the current timestamp
    high_priority_pcb.execution_starttime = timestamp;

    // Set the execution end time as the sum of the current timestamp and the remaining burst time
    high_priority_pcb.execution_endtime = timestamp + high_priority_pcb.remaining_bursttime;
    
    // Shift remaining PCBs in the ready queue
    for (int i = high_priority_index; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    
    // Decrement queue count
    (*queue_cnt)--;

    return high_priority_pcb;
}



struct PCB handle_process_arrival_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    new_process.remaining_bursttime = new_process.total_bursttime;

    // If there is no currently-running process (i.e., the third argument is the NULLPCB)
    if (current_process.process_id == 0) {
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        return new_process;
    } else {
        // If there is a currently-running process
        if (new_process.total_bursttime < current_process.remaining_bursttime) {
            // If the new process has a shorter burst time
            current_process.remaining_bursttime -= timestamp - current_process.execution_starttime;
            current_process.execution_starttime = 0; 
            current_process.execution_endtime = 0;
            ready_queue[*queue_cnt] = current_process; 
            (*queue_cnt)++;
            new_process.execution_starttime = timestamp;
            new_process.execution_endtime = timestamp + new_process.total_bursttime;
            return new_process;
        } else {
            // If the new process does not have a shorter burst time
            new_process.execution_starttime = 0;
            new_process.execution_endtime = 0;
            ready_queue[*queue_cnt] = new_process;
            (*queue_cnt)++;
            return current_process;
        }
    }
}


struct PCB handle_process_completion_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    int nextProcessIndex = 0;
    if (*queue_cnt == 0) {
        struct PCB nullpcb = {0, 0, 0, 0, 0, 0};
        return nullpcb;
    } else {
        for (int i = 1; i < *queue_cnt; i++) {
            if (ready_queue[i].remaining_bursttime < ready_queue[nextProcessIndex].remaining_bursttime) {
                nextProcessIndex = i;
            }
        }
        struct PCB nextProcess = ready_queue[nextProcessIndex];
        // update execution_starttime and execution_endtime
        nextProcess.execution_starttime = timestamp;
        nextProcess.execution_endtime = timestamp + nextProcess.total_bursttime;
        for (int i = nextProcessIndex; i < *queue_cnt - 1; i++) {
            ready_queue[i] = ready_queue[i+1];
        }
        (*queue_cnt)--;
        return nextProcess;
    }
}


void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt) {
    struct MEMORY_BLOCK temp_map[MAPMAX];
    int temp_map_cnt = 0;
    int merged = 0;

    // Iterate over memory_map
    for (int i = 0; i < *map_cnt; i++) {
        // If the current block is adjacent to the freed block and it is not in use
        if ((memory_map[i].end_address + 1 == freed_block.start_address || memory_map[i].start_address - 1 == freed_block.end_address) && memory_map[i].process_id == 0) {
            // Merge the two blocks
            temp_map[temp_map_cnt].start_address = freed_block.start_address < memory_map[i].start_address ? freed_block.start_address : memory_map[i].start_address;
            temp_map[temp_map_cnt].end_address = freed_block.end_address > memory_map[i].end_address ? freed_block.end_address : memory_map[i].end_address;
            temp_map[temp_map_cnt].process_id = 0;
            temp_map[temp_map_cnt].segment_size = temp_map[temp_map_cnt].end_address - temp_map[temp_map_cnt].start_address + 1;
            temp_map_cnt++;
            merged = 1;
        } else {
            // Keep the block as it is
            temp_map[temp_map_cnt++] = memory_map[i];
        }
    }

    // If the freed block wasn't merged with any block, add it to temp_map
    if (!merged) {
        freed_block.process_id = 0;
        freed_block.segment_size = freed_block.end_address - freed_block.start_address + 1;
        temp_map[temp_map_cnt++] = freed_block;
    }

    // Copy temp_map to memory_map
    for (int i = 0; i < temp_map_cnt; i++) {
        memory_map[i] = temp_map[i];
    }

    *map_cnt = temp_map_cnt;
}

int process_page_access_fifo(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    struct PTE page_entry = page_table[page_number];

    if (page_entry.is_valid) {
        return page_entry.frame_number;
    } else {
        page_entry.is_valid = 1;
        page_entry.frame_number = frame_pool[(*frame_cnt)++]; // assuming frame_pool has enough frames
        page_entry.arrival_timestamp = current_timestamp;
        page_entry.last_access_timestamp = current_timestamp;
        page_table[page_number] = page_entry;
        return page_entry.frame_number;
    }
}

struct RCB handle_request_arrival_fcfs(struct RCB request_queue[QUEUEMAX],int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp) {
    // Insert the new request at the end of the queue
    new_request.arrival_timestamp = timestamp;
    request_queue[*queue_cnt] = new_request;
    (*queue_cnt)++;

    // If there is no current request, start servicing the new request
    if(current_request.request_id == -1)
        return new_request;

    return current_request;
}

struct RCB handle_request_completion_fcfs(struct RCB request_queue[QUEUEMAX],int *queue_cnt) {
    struct RCB completed_request = request_queue[0];

    // Shift the queue to remove the completed request
    for(int i=0; i<*queue_cnt-1; i++) {
        request_queue[i] = request_queue[i+1];
    }
    (*queue_cnt)--;

    // If there are more requests, the next one becomes the current
    if(*queue_cnt > 0) {
        return request_queue[0];
    }

    // If there are no more requests, return an empty request
    struct RCB empty_request = { .request_id = -1 };
    return empty_request;
}


struct RCB handle_request_arrival_sstf(struct RCB request_queue[QUEUEMAX],int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp) {
    // Insert the new request at the end of the queue
    new_request.arrival_timestamp = timestamp;
    request_queue[*queue_cnt] = new_request;
    (*queue_cnt)++;

    return current_request;
}

struct RCB handle_request_completion_sstf(struct RCB request_queue[QUEUEMAX],int *queue_cnt,int current_cylinder) {
    int shortest_seek_time = INT_MAX;
    int shortest_seek_time_index = -1;

    // Search the queue to find the request with the shortest seek time
    for(int i=0; i< *queue_cnt; i++) {
        int seek_time = abs(current_cylinder - request_queue[i].cylinder);
        if(seek_time < shortest_seek_time) {
            shortest_seek_time = seek_time;
            shortest_seek_time_index = i;
        }
    }

    struct RCB completed_request = request_queue[shortest_seek_time_index];

    // Remove the completed request from the queue
    for(int i=shortest_seek_time_index; i<*queue_cnt-1; i++) {
        request_queue[i] = request_queue[i+1];
    }
    (*queue_cnt)--;

    return completed_request;
}


struct RCB handle_request_arrival_look(struct RCB request_queue[QUEUEMAX],int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp) {
    // insert new request at the end of the queue
    new_request.arrival_timestamp = timestamp;
    request_queue[*queue_cnt] = new_request;
    (*queue_cnt)++;

    return current_request;
}

struct RCB handle_request_completion_look(struct RCB request_queue[QUEUEMAX],int *queue_cnt, int current_cylinder, int scan_direction) {
    int index = -1;
    for(int i=0; i< *queue_cnt; i++) {
        // for forward scan
        if(scan_direction > 0 && request_queue[i].cylinder >= current_cylinder) {
            index = i;
            break;
        }
        // for backward scan
        else if(scan_direction < 0 && request_queue[i].cylinder <= current_cylinder) {
            index = i;
            break;
        }
    }

    struct RCB completed_request;
    if(index != -1) {
        completed_request = request_queue[index];

        // remove completed request from queue
        for(int i=index; i<*queue_cnt-1; i++) {
            request_queue[i] = request_queue[i+1];
        }
        (*queue_cnt)--;
    }

    return completed_request;
}


struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt, int process_id) {
    int min_size = INT_MAX;
    int min_index = -1;
    for(int i=0; i< *map_cnt; i++) {
        if(memory_map[i].segment_size >= request_size && memory_map[i].segment_size < min_size && memory_map[i].process_id == 0) {
            min_size = memory_map[i].segment_size;
            min_index = i;
        }
    }

    struct MEMORY_BLOCK null_block = {0, 0, 0, 0};
    if(min_index == -1) {
        return null_block;
    }

    struct MEMORY_BLOCK allocated_block = {memory_map[min_index].start_address, memory_map[min_index].start_address + request_size - 1, request_size, process_id};
    memory_map[min_index].start_address += request_size;
    memory_map[min_index].segment_size -= request_size;

    if(memory_map[min_index].segment_size == 0) {
        for(int i=min_index; i<*map_cnt-1; i++) {
            memory_map[i] = memory_map[i+1];
        }
        (*map_cnt)--;
    }

    return allocated_block;
}

struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt, int process_id) {
    int found_index = -1;
    for(int i=0; i< *map_cnt; i++) {
        if(memory_map[i].segment_size >= request_size && memory_map[i].process_id == 0) {
            found_index = i;
            break;
        }
    }

    struct MEMORY_BLOCK null_block = {0, 0, 0, 0};
    if(found_index == -1) {
        return null_block;
    }

    struct MEMORY_BLOCK allocated_block = {memory_map[found_index].start_address, memory_map[found_index].start_address + request_size - 1, request_size, process_id};
    memory_map[found_index].start_address += request_size;
    memory_map[found_index].segment_size -= request_size;

    if(memory_map[found_index].segment_size == 0) {
        for(int i=found_index; i<*map_cnt-1; i++) {
            memory_map[i] = memory_map[i+1];
        }
        (*map_cnt)--;
    }

    return allocated_block;
}


// worst_fit_allocate()
struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    int largest_block_index = -1;
    int largest_block_size = 0;
    int i;

    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].segment_size >= request_size && memory_map[i].segment_size > largest_block_size && memory_map[i].process_id == 0) {
            largest_block_size = memory_map[i].segment_size;
            largest_block_index = i;
        }
    }

    if (largest_block_index == -1) {
        return (struct MEMORY_BLOCK){0, 0, 0, 0};
    }

    if (memory_map[largest_block_index].segment_size == request_size) {
        memory_map[largest_block_index].process_id = process_id;
        return memory_map[largest_block_index];
    } else {
        struct MEMORY_BLOCK new_block = {memory_map[largest_block_index].start_address, memory_map[largest_block_index].start_address + request_size - 1, process_id, request_size};
        memory_map[largest_block_index].start_address += request_size;
        memory_map[largest_block_index].segment_size -= request_size;
        memory_map[*map_cnt] = new_block;
        (*map_cnt)++;
        return new_block;
    }
}

// next_fit_allocate()
struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address) {
    int i;

    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].start_address >= last_address && memory_map[i].segment_size >= request_size && memory_map[i].process_id == 0) {
            if (memory_map[i].segment_size == request_size) {
                memory_map[i].process_id = process_id;
                return memory_map[i];
            } else {
                struct MEMORY_BLOCK new_block = {memory_map[i].start_address, memory_map[i].start_address + request_size - 1, process_id, request_size};
                memory_map[i].start_address += request_size;
                memory_map[i].segment_size -= request_size;
                memory_map[*map_cnt] = new_block;
                (*map_cnt)++;
                return new_block;
            }
        }
    }

    // if no suitable block is found after the last_address, start from the beginning
    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].segment_size >= request_size && memory_map[i].process_id == 0) {
            if (memory_map[i].segment_size == request_size) {
                memory_map[i].process_id = process_id;
                return memory_map[i];
            } else {
                struct MEMORY_BLOCK new_block = {memory_map[i].start_address, memory_map[i].start_address + request_size - 1, process_id, request_size};
                memory_map[i].start_address += request_size;
                memory_map[i].segment_size -= request_size;
                memory_map[*map_cnt] = new_block;
                (*map_cnt)++;
                return new_block;
            }
        }
    }

    return (struct MEMORY_BLOCK){0, 0, 0, 0}; // no suitable block found, return an error
}

int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int refrence_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = refrence_string[i];
        struct PTE page_entry = page_table[page_number];

        if (!page_entry.is_valid) {
            page_faults++; // it's a page fault
            page_entry.is_valid = 1;
            page_entry.frame_number = frame_pool[frame_cnt++]; // assuming frame_pool has enough frames
            page_entry.arrival_timestamp = i;
            page_entry.last_access_timestamp = i;
            page_table[page_number] = page_entry;
        }
    }

    return page_faults;
}

int process_page_access_lru(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    struct PTE page_entry = page_table[page_number];

    if (page_entry.is_valid) {
        page_entry.last_access_timestamp = current_timestamp; // updating the access timestamp
        page_table[page_number] = page_entry;
        return page_entry.frame_number;
    } else {
        page_entry.is_valid = 1;
        page_entry.frame_number = frame_pool[(*frame_cnt)++]; // assuming frame_pool has enough frames
        page_entry.arrival_timestamp = current_timestamp;
        page_entry.last_access_timestamp = current_timestamp;
        page_table[page_number] = page_entry;
        return page_entry.frame_number;
    }
}

int count_page_faults_lru(struct PTE page_table[TABLEMAX], int table_cnt, int refrence_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = refrence_string[i];
        struct PTE page_entry = page_table[page_number];

        if (page_entry.is_valid) {
            page_entry.last_access_timestamp = i; // updating the access timestamp
            page_table[page_number] = page_entry;
        } else {
            page_faults++; // it's a page fault
            page_entry.is_valid = 1;
            page_entry.frame_number = frame_pool[frame_cnt++]; // assuming frame_pool has enough frames
            page_entry.last_access_timestamp = i;
            page_table[page_number] = page_entry;
        }
    }

    return page_faults;
}

int process_page_access_lfu(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    struct PTE page_entry = page_table[page_number];

    if (page_entry.is_valid) {
        page_entry.reference_count++; // increment the reference count
        page_table[page_number] = page_entry;
        return page_entry.frame_number;
    } else {
        page_entry.is_valid = 1;
        page_entry.frame_number = frame_pool[(*frame_cnt)++]; // assuming frame_pool has enough frames
        page_entry.arrival_timestamp = current_timestamp;
        page_entry.last_access_timestamp = current_timestamp;
        page_entry.reference_count = 1; // initial reference count
        page_table[page_number] = page_entry;
        return page_entry.frame_number;
    }
}

int count_page_faults_lfu(struct PTE page_table[TABLEMAX], int table_cnt, int refrence_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = refrence_string[i];
        struct PTE page_entry = page_table[page_number];

        if (page_entry.is_valid) {
            page_entry.reference_count++; // increment the reference count
            page_table[page_number] = page_entry;
        } else {
            page_faults++; // it's a page fault
            page_entry.is_valid = 1;
            page_entry.frame_number = frame_pool[frame_cnt++]; // assuming frame_pool has enough frames
            page_entry.arrival_timestamp = i;
            page_entry.last_access_timestamp = i;
            page_entry.reference_count = 1;
            page_table[page_number] = page_entry;
        }
    }

    return page_faults;
}


struct PCB handle_process_arrival_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp, int time_quantum) {
    new_process.remaining_bursttime = new_process.total_bursttime;

    // If there is no currently-running process (i.e., the third argument is the NULLPCB)
    if (current_process.process_id == 0) {
        new_process.execution_starttime = timestamp;
        new_process.execution_endtime = timestamp + ((new_process.total_bursttime < time_quantum) ? new_process.total_bursttime : time_quantum);
        return new_process;
    } else {
        // If there is a currently-running process
        new_process.execution_starttime = 0;
        new_process.execution_endtime = 0;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
        return current_process;
    }
}


struct PCB handle_process_completion_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp, int time_quantum) {
    // If the ready queue is empty, return NULLPCB
    if (*queue_cnt == 0) {
        return NULLPCB;
    }

    // Initialize earliest arrival time variable and index of the earliest arrival process
    int earliest_arrival = INT_MAX;
    int earliest_arrival_index = 0;

    // Find the PCB of the process in the ready queue with the earliest arrival time
    for (int i = 0; i < *queue_cnt; i++) {
        if (ready_queue[i].arrival_timestamp < earliest_arrival) {
            earliest_arrival = ready_queue[i].arrival_timestamp;
            earliest_arrival_index = i;
        }
    }

    // Save the earliest arrival PCB
    struct PCB earliest_arrival_pcb = ready_queue[earliest_arrival_index];
    
    // Set the execution start time as the current timestamp
    earliest_arrival_pcb.execution_starttime = timestamp;

    // Set the execution end time as the sum of the current timestamp and the smaller of the time quantum and the remaining burst time
    if (earliest_arrival_pcb.remaining_bursttime < time_quantum) {
        earliest_arrival_pcb.execution_endtime = timestamp + earliest_arrival_pcb.remaining_bursttime;
    } else {
        earliest_arrival_pcb.execution_endtime = timestamp + time_quantum;
    }
    
    // Shift remaining PCBs in the ready queue
    for (int i = earliest_arrival_index; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    
    // Decrement queue count
    (*queue_cnt)--;

    return earliest_arrival_pcb;
}


// int main() {
//     struct PCB ready_queue[QUEUEMAX];
//     int queue_cnt = 0;
//     struct PCB current_process = {0, 0, 0, 0, 0, 0, 0};
//     struct PCB new_process = {5, 2, 5, 0, 0, 5, 4};
//     int timestamp = 2;

//     struct PCB next_process = handle_process_arrival_pp(ready_queue, &queue_cnt, current_process, new_process, timestamp);

//     // Output the result
//     printf("Next process to execute - PID: %d\n", next_process.process_id);
//     printf("Ready queue count: %d\n", queue_cnt);
//     for (int i = 0; i < queue_cnt; i++) {
//         printf("Ready queue - PID: %d, AT: %d, TBT: %d, EST: %d, EET: %d, RBT: %d, Priority: %d\n",
//                ready_queue[i].process_id, ready_queue[i].arrival_timestamp,
//                ready_queue[i].total_bursttime, ready_queue[i].execution_starttime,
//                ready_queue[i].execution_endtime, ready_queue[i].remaining_bursttime,
//                ready_queue[i].process_priority);
//     }

//     return 0;
// }


// int main() {
//     struct PCB ready_queue[QUEUEMAX]; // Assuming QUEUEMAX is a defined constant - This is defined in the header file oslabs.h 
//     int queue_cnt = 0;
//     struct PCB current_process, new_process;

//     // Setting up the current process
//     current_process.process_id = 1;
//     current_process.arrival_timestamp = 1;
//     current_process.total_bursttime = 4;
//     current_process.execution_starttime = 1;
//     current_process.execution_endtime = 0; // Endtime should be 0 for a running process
//     current_process.remaining_bursttime = 3;
//     current_process.process_priority = 8;

//     // Setting up the new process
//     new_process.process_id = 2;
//     new_process.arrival_timestamp = 2;
//     new_process.total_bursttime = 3;
//     new_process.execution_starttime = 0;
//     new_process.execution_endtime = 0; // Endtime should be 0 for a new process
//     new_process.remaining_bursttime = 3;
//     new_process.process_priority = 6; // Higher priority than current_process

//     // Calling the function to test
//     struct PCB next_process = handle_process_arrival_pp(ready_queue, &queue_cnt, current_process, new_process, 2);

//     // Here you would typically check if the ready_queue and queue_cnt are as expected
//     if(queue_cnt == 1) {
//         printf("Passed All Tests\n");
//     } else {
//         printf("Expecting test 1 Ready Queue Count value of 1 and received %d\n", queue_cnt);
//     }

//     return 0;
// }

// int main() {
//     struct PCB ready_queue[QUEUEMAX];
//     int queue_cnt = 0;
//     struct PCB process;

//     // Test handle_process_arrival_pp
//     process = (struct PCB){ .process_id = 1, .arrival_timestamp = 1, .total_bursttime = 4, .execution_starttime = 1, .execution_endtime = 2, .remaining_bursttime = 3, .process_priority = 8 };
    
//     /* Your Function Call Here */

//     if(queue_cnt != 1) {
//         printf("Test Failed for handle_process_arrival_pp\n");
//     } else {
//         printf("Test Passed for handle_process_arrival_pp\n");
//     }

//     // Test handle_process_arrival_rr
//     process = (struct PCB){ .process_id = 5, .arrival_timestamp = 2, .total_bursttime = 5, .execution_starttime = 0, .execution_endtime = 0, .remaining_bursttime = 5, .process_priority = 4 };
    
//     /* Your Function Call Here */

//     if(queue_cnt != 0) {
//         printf("Test Failed for handle_process_arrival_rr\n");
//     } else {
//         printf("Test Passed for handle_process_arrival_rr\n");
//     }

//     // Test handle_process_arrival_srtp
//     process = (struct PCB){ .process_id = 1, .arrival_timestamp = 1, .total_bursttime = 8, .execution_starttime = 1, .execution_endtime = 0, .remaining_bursttime = 7, .process_priority = 0 };
    
//     /* Your Function Call Here */

//     if(queue_cnt != 1) {
//         printf("Test Failed for handle_process_arrival_srtp\n");
//     } else {
//         printf("Test Passed for handle_process_arrival_srtp\n");
//     }

//     // Test handle_process_completion_pp
//     queue_cnt = 2;
//     ready_queue[0] = (struct PCB){ .process_id = 2, .arrival_timestamp = 1, .total_bursttime = 4, .execution_starttime = 0, .execution_endtime = 0, .remaining_bursttime = 4, .process_priority = 22 };
//     ready_queue[1] = (struct PCB){ .process_id = 3, .arrival_timestamp = 1, .total_bursttime = 4, .execution_starttime = 0, .execution_endtime = 0, .remaining_bursttime = 4, .process_priority = 24 };

//     /* Your Function Call Here */

//     if(queue_cnt != 1) {
//         printf("Test Failed for handle_process_completion_pp\n");
//     } else {
//         printf("Test Passed for handle_process_completion_pp\n");
//     }

//      // Test handle_process_completion_rr
//     ready_queue[0] = (struct PCB){ .process_id = 2, .arrival_timestamp = 21, .total_bursttime = 8, .execution_starttime = 0, .execution_endtime = 0, .remaining_bursttime = 8, .process_priority = 0 };
//     ready_queue[1] = (struct PCB){ .process_id = 3, .arrival_timestamp = 23, .total_bursttime = 8, .execution_starttime = 0, .execution_endtime = 0, .remaining_bursttime = 8, .process_priority = 0 };

//     /* Your Function Call Here */

//     if(queue_cnt != 1) {
//         printf("Test Failed for handle_process_completion_rr\n");
//     } else {
//         printf("Test Passed for handle_process_completion_rr\n");
//     }

//     return 0;
// }
