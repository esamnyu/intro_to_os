#include "oslabs.h"
#include <limits.h>
#include <stdlib.h>

const struct PCB NULLPCB = {0, 0, 0, 0, 0, 0, 0};

struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    new_process.execution_starttime = timestamp;
    new_process.remaining_bursttime = new_process.total_bursttime;

    if (current_process.process_id == 0) { // Assuming process_id = 0 means it's the NULLPCB
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
        return new_process;
    } else {
        if (new_process.process_priority > current_process.process_priority) { // New process preempts the current one
            current_process.remaining_bursttime -= timestamp - current_process.execution_starttime;
            current_process.execution_endtime = timestamp + current_process.remaining_bursttime;
            ready_queue[*queue_cnt] = current_process; // current_process is added back to ready_queue
            (*queue_cnt)++;
            new_process.execution_endtime = timestamp + new_process.total_bursttime;
            ready_queue[*queue_cnt] = new_process; // new_process is added to ready_queue
            (*queue_cnt)++;
            return new_process;
        } else { // Current process continues
            new_process.execution_endtime = current_process.execution_endtime + new_process.total_bursttime;
            ready_queue[*queue_cnt] = new_process;
            (*queue_cnt)++;
            return current_process; // current_process continues its execution
        }
    }
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

struct PCB handle_process_arrival_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int time_stamp) {
    if(current_process.process_id == 0 || new_process.remaining_bursttime < current_process.remaining_bursttime) {
        new_process.execution_starttime = time_stamp;
        new_process.execution_endtime = time_stamp + new_process.remaining_bursttime;
        
        if(current_process.process_id != 0) {
            current_process.execution_endtime = 0;
            ready_queue[*queue_cnt] = current_process;
            (*queue_cnt)++;
        }
        
        return new_process;
    } else {
        new_process.execution_starttime = 0;
        new_process.execution_endtime = 0;
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
        
        return current_process;
    }
}

struct PCB handle_process_completion_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    // We start by assuming that the first process in the queue will be the next
    int nextProcessIndex = 0;
    for (int i = 1; i < *queue_cnt; i++) {
        // If we find a process with smaller remaining burst time, we update nextProcessIndex
        if (ready_queue[i].remaining_bursttime < ready_queue[nextProcessIndex].remaining_bursttime) {
            nextProcessIndex = i;
        }
    }

    struct PCB next_process = ready_queue[nextProcessIndex];

    // remove next_process from ready_queue and shift all other processes
    for (int i = nextProcessIndex; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }

    // Decrease the queue count
    (*queue_cnt)--;
    
    return next_process;
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
    // If a new process arrives, it will be added to the end of the ready queue
    ready_queue[*queue_cnt] = new_process;
    (*queue_cnt)++; // increment the queue count

    // If the current process has exceeded its time quantum, it should be added to the end of the queue
    if (timestamp - current_process.execution_starttime >= time_quantum) {
        ready_queue[*queue_cnt] = current_process;
        (*queue_cnt)++; // increment the queue count

        // The new current process is the one that was at the head of the queue
        struct PCB next_process = ready_queue[0];

        // remove the process from the head of the queue and shift all other processes
        for (int i = 0; i < *queue_cnt - 1; i++) {
            ready_queue[i] = ready_queue[i + 1];
        }

        (*queue_cnt)--; // decrement the queue count
        return next_process;
    }

    // If the current process has not exhausted its quantum, it will continue to execute
    return current_process;
}

struct PCB handle_process_completion_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp, int time_quantum) {
    // Assuming the process at the head of the queue is the one that just completed or exhausted its time quantum
    struct PCB completed_process = ready_queue[0];

    // We need to update its remaining burst time if it is not yet completed
    int elapsed_time = timestamp - completed_process.execution_starttime;
    if (elapsed_time < completed_process.remaining_bursttime) {
        completed_process.remaining_bursttime -= elapsed_time;
    } else {
        completed_process.remaining_bursttime = 0;
    }

    // Shift all the other processes in the queue
    for (int i = 0; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }

    (*queue_cnt)--; // Decrease the queue count

    // If the completed process is not really completed (it just exhausted its time quantum), add it to the end of the queue
    if (completed_process.remaining_bursttime > 0) {
        ready_queue[*queue_cnt] = completed_process;
        (*queue_cnt)++;
    }

    // The new current process is the one now at the head of the queue
    struct PCB next_process = ready_queue[0];
    return next_process;
}
// Is not required for this assignment  
// int main() {
//     return 0;
// }