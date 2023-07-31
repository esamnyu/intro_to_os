#include <stdio.h>
#include <stdlib.h>
#include "oslabs.h"

int process_page_access_fifo(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp) {
    if (page_table[page_number].is_valid == 1) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count += 1;
        return page_table[page_number].frame_number;
    }
    else if (*frame_cnt != 0) {
        int free_frame_index;
        for (int i = 0; i < *frame_cnt; i++) {
            if (frame_pool[i] != 0) {
                free_frame_index = i;
                break;
            }
        }
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].frame_number = frame_pool[free_frame_index];
        page_table[page_number].is_valid = 1;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        // remove frame from pool
        frame_pool[free_frame_index] = 0;
        (*frame_cnt)--;
        // return the frame number associated with the page-table entry
        return page_table[page_number].frame_number;
    }
    // else if the page being referenced is not in memory and there are no free frames for the process
    else {
        // a page needs to be replaced
        // select among all the pages of the process that are currently in memory
        // int to keep track of smallest index
        int smallest_arrival_index = 0;
        // find first smallest
        // loop through page table
        for (int i = 0; i < *table_cnt; i++) {
            // if the arrival timestamp is less than the current smallest
            if (page_table[i].is_valid == 1) {
                // update index to newest smallest
                smallest_arrival_index = i;
                break;
            }
        }            
        // loop through page table
        for (int i = smallest_arrival_index + 1; i < *table_cnt; i++) {
            // if the arrival timestamp is less than the current smallest
            if (page_table[i].is_valid == 1 && page_table[i].arrival_timestamp < page_table[smallest_arrival_index].arrival_timestamp) {
                // update index to newest smallest
                smallest_arrival_index = i;
            }
        }
        //printf("smallest arrival %d", smallest_arrival_index);
        // choose the page that has the smallest arrival_timestamp
        // mark page_table entry as invalid
        page_table[smallest_arrival_index].is_valid = 0;
        // set the frame_number of the page-table entry of the newly-referenced page to the newly freed frame
        page_table[page_number].frame_number = page_table[smallest_arrival_index].frame_number;
        // set the frame_number to -1
        page_table[smallest_arrival_index].frame_number = -1;
        // set the arrival_timestamp to -1
        page_table[smallest_arrival_index].arrival_timestamp = -1;
        // set the reference_count to -1
        page_table[smallest_arrival_index].reference_count = -1;
        // set the last_access_timestamp to -1
        page_table[smallest_arrival_index].last_access_timestamp = -1;
        // set arrival time_stamp
        page_table[page_number].arrival_timestamp = current_timestamp;
        // set is_valid
        page_table[page_number].is_valid = 1;
        // set last_access_timestamp
        page_table[page_number].last_access_timestamp = current_timestamp;
        //set reference_count
        page_table[page_number].reference_count = 1;
        //printf("test 3");
        // return frame number
        return page_table[page_number].frame_number;
    }
}

// simulates the processing of a sequence of page references in a system that uses the First-In-First-Out (FIFO) policy for page replacement
int count_page_faults_fifo(struct PTE page_table[TABLEMAX],int table_cnt, int refrence_string[REFERENCEMAX],int reference_cnt,int frame_pool[POOLMAX],int frame_cnt) {
    // simulate timestamps
    int current_timestamp = 1;
    // count number of faults
    int fault_count = 0;
    // check if page being referenced is already in memory
    for (int i = 0; i < reference_cnt; i++) {
        current_timestamp++;
        int page_number = refrence_string[i];
        // first check if the page being referenced is already in memory
        if (page_table[page_number].is_valid == 1) {
            // modify last_access_timestamp
            page_table[page_number].last_access_timestamp = current_timestamp;
            // modify reference_count field
            page_table[page_number].reference_count += 1;
        }
        // else if the page being referenced is not in memory
        // check if there are free frames, the process frame pool is not empty
        else {
            fault_count++;
            if (frame_cnt != 0) {
                // get first free frame 
                int free_frame_index;
                // remove a frame from the process frame pool
                for (int i = 0; i < frame_cnt; i++) {
                    if (frame_pool[i] != 0) {
                        free_frame_index = i;
                        break;
                    }
                }
                // frame number is inserted into the page-table entry corresponding to the logical page number
                // update values accordingly
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].frame_number = frame_pool[free_frame_index];
                page_table[page_number].is_valid = 1;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
                // remove frame from pool
                frame_pool[free_frame_index] = 0;
                (frame_cnt)--;
            }
            // else if the page being referenced is not in memory and there are no free frames for the process
            else {
                // a page needs to be replaced
                // select among all the pages of the process that are currently in memory
                // int to keep track of smallest index
                int smallest_arrival_index = 0;
                // find first smallest
                // loop through page table
                for (int i = 0; i < table_cnt; i++) {
                    // if the arrival timestamp is less than the current smallest
                    if (page_table[i].is_valid == 1) {
                        // update index to newest smallest
                        smallest_arrival_index = i;
                        break;
                    }
                }            
                // loop through page table
                for (int i = smallest_arrival_index + 1; i < table_cnt; i++) {
                    // if the arrival timestamp is less than the current smallest
                    if (page_table[i].is_valid == 1 && page_table[i].arrival_timestamp < page_table[smallest_arrival_index].arrival_timestamp) {
                        // update index to newest smallest
                        smallest_arrival_index = i;
                    }
                }
                page_table[smallest_arrival_index].is_valid = 0;
                page_table[page_number].frame_number = page_table[smallest_arrival_index].frame_number;
                page_table[smallest_arrival_index].frame_number = -1;
                page_table[smallest_arrival_index].arrival_timestamp = -1;
                page_table[smallest_arrival_index].reference_count = -1;
                page_table[smallest_arrival_index].last_access_timestamp = -1;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].is_valid = 1;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            }
        }
    }
    return fault_count;
}

// implements the logic to process a page access in a system that uses the Least-Recently-Used (LRU) policy for page replacement
int process_page_access_lru(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp) {
    // first check if the page being referenced is already in memory
    if (page_table[page_number].is_valid == 1) {
        // modify last_access_timestamp
        page_table[page_number].last_access_timestamp = current_timestamp;
        // modify reference_count field
        page_table[page_number].reference_count += 1;
        // return the frame number
        return page_table[page_number].frame_number;
    }
    // else if the page being referenced is not in memory
    // check if there are free frames, the process frame pool is not empty
    else if (*frame_cnt != 0) {
        // get first free frame 
        int free_frame_index;
        // remove a frame from the process frame pool
        for (int i = 0; i < *frame_cnt; i++) {
            if (frame_pool[i] != 0) {
                free_frame_index = i;
                break;
            }
        }
        // frame number is inserted into the page-table entry corresponding to the logical page number
        // update values accordingly
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].frame_number = frame_pool[free_frame_index];
        page_table[page_number].is_valid = 1;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        // remove frame from pool
        frame_pool[free_frame_index] = 0;
        (*frame_cnt)--;
        // return the frame number associated with the page-table entry
        return page_table[page_number].frame_number;
    }
    // else if the page being referenced is not in memory and there are no free frames for the process
    else {
        // a page needs to be replaced
        // select among all the pages of the process that are currently in memory
        // select the smallest last_acess
        int smallest_last_access_index = 0;
        // find first smallest
        // loop through page table
        for (int i = 0; i < *table_cnt; i++) {
            // if the arrival timestamp is less than the current smallest
            if (page_table[i].is_valid == 1) {
                // update index to newest smallest
                smallest_last_access_index = i;
                break;
            }
        }            
        // loop through page table
        for (int i = smallest_last_access_index + 1; i < *table_cnt; i++) {
            // if the arrival timestamp is less than the current smallest
            if (page_table[i].is_valid == 1 && page_table[i].last_access_timestamp < page_table[smallest_last_access_index].last_access_timestamp) {
                // update index to newest smallest
                smallest_last_access_index = i;
            }
        }
        // choose the page that has the smallest last_access_timestamp
        // mark page_table entry as invalid
        page_table[smallest_last_access_index].is_valid = 0;
        // set the frame_number of the page-table entry of the newly-referenced page to the newly freed frame
        page_table[page_number].frame_number = page_table[smallest_last_access_index].frame_number;
        // set the frame_number to -1
        page_table[smallest_last_access_index].frame_number = -1;
        // set the arrival_timestamp to -1
        page_table[smallest_last_access_index].arrival_timestamp = -1;
        // set the reference_count to -1
        page_table[smallest_last_access_index].reference_count = -1;
        // set the last_access_timestamp to -1
        page_table[smallest_last_access_index].last_access_timestamp = -1;
        // set arrival time_stamp
        page_table[page_number].arrival_timestamp = current_timestamp;
        // set is_valid
        page_table[page_number].is_valid = 1;
        // set last_access_timestamp
        page_table[page_number].last_access_timestamp = current_timestamp;
        //set reference_count
        page_table[page_number].reference_count = 1;
        // return frame number
        return page_table[page_number].frame_number;
    }    
}

// simulates the processing of a sequence of page references in a system that uses the Least-Recently-Used (LRU) policy for page replacement
int count_page_faults_lru(struct PTE page_table[TABLEMAX],int table_cnt, int refrence_string[REFERENCEMAX],int reference_cnt,int frame_pool[POOLMAX],int frame_cnt) {
    // simulate timestamps
    int current_timestamp = 1;
    // count number of faults
    int fault_count = 0;
    // check if page being referenced is already in memory
    for (int i = 0; i < reference_cnt; i++) {
        current_timestamp++;
        int page_number = refrence_string[i];
        // first check if the page being referenced is already in memory
        if (page_table[page_number].is_valid == 1) {
            // modify last_access_timestamp
            page_table[page_number].last_access_timestamp = current_timestamp;
            // modify reference_count field
            page_table[page_number].reference_count += 1;
        }
        // else if the page being referenced is not in memory
        // check if there are free frames, the process frame pool is not empty
        else {
            fault_count++;
            if (frame_cnt != 0) {
            // get first free frame 
            int free_frame_index;
            // remove a frame from the process frame pool
            for (int i = 0; i < frame_cnt; i++) {
                if (frame_pool[i] != 0) {
                    free_frame_index = i;
                    break;
                }
            }
            // frame number is inserted into the page-table entry corresponding to the logical page number
            // update values accordingly
            page_table[page_number].arrival_timestamp = current_timestamp;
            page_table[page_number].frame_number = frame_pool[free_frame_index];
            page_table[page_number].is_valid = 1;
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count = 1;
            // remove frame from pool
            frame_pool[free_frame_index] = 0;
            (frame_cnt)--;
            }
            // else if the page being referenced is not in memory and there are no free frames for the process
            else {
                // a page needs to be replaced
                // select among all the pages of the process that are currently in memory
                // select the smallest last_acess
                int smallest_last_access_index = 0;
                // find first smallest
                // loop through page table
                for (int i = 0; i < table_cnt; i++) {
                    // if the arrival timestamp is less than the current smallest
                    if (page_table[i].is_valid == 1) {
                        // update index to newest smallest
                        smallest_last_access_index = i;
                        break;
                    }
                }            
                // loop through page table
                for (int i = smallest_last_access_index + 1; i < table_cnt; i++) {
                    // if the arrival timestamp is less than the current smallest
                    if (page_table[i].is_valid == 1 && page_table[i].last_access_timestamp < page_table[smallest_last_access_index].last_access_timestamp) {
                        // update index to newest smallest
                        smallest_last_access_index = i;
                    }
                }
                page_table[smallest_last_access_index].is_valid = 0;
                page_table[page_number].frame_number = page_table[smallest_last_access_index].frame_number;
                page_table[smallest_last_access_index].frame_number = -1;
                page_table[smallest_last_access_index].arrival_timestamp = -1;
                page_table[smallest_last_access_index].reference_count = -1;
                page_table[smallest_last_access_index].last_access_timestamp = -1;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].is_valid = 1;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;
            }   
        }
    }   
    return fault_count; 
}

int process_page_access_lfu(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp) {
    if (page_table[page_number].is_valid == 1) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count += 1;
        return page_table[page_number].frame_number;
    }
    else if (*frame_cnt != 0) {
        int free_frame_index;
        for (int i = 0; i < *frame_cnt; i++) {
            if (frame_pool[i] != 0) {
                free_frame_index = i;
                break;
            }
        }
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].frame_number = frame_pool[free_frame_index];
        page_table[page_number].is_valid = 1;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        frame_pool[free_frame_index] = 0;
        (*frame_cnt)--;
        return page_table[page_number].frame_number;
    }
    else {
        int smallest_reference_count = 0;
            if (page_table[i].is_valid == 1) {
                smallest_reference_count = i;
                break;
            }
        }  
        for (int i = smallest_reference_count + 1; i < *table_cnt; i++) {
            if (page_table[i].is_valid == 1 && page_table[i].reference_count <= page_table[smallest_reference_count].reference_count) {
                smallest_reference_count = i;
            }
        }
        page_table[smallest_reference_count].is_valid = 0;
        page_table[page_number].frame_number = page_table[smallest_reference_count].frame_number;
        page_table[smallest_reference_count].frame_number = -1;
        page_table[smallest_reference_count].arrival_timestamp = -1;
        page_table[smallest_reference_count].reference_count = -1;
        page_table[smallest_reference_count].last_access_timestamp = -1;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].is_valid = 1;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        return page_table[page_number].frame_number;
    }   
}

int count_page_faults_lfu(struct PTE page_table[TABLEMAX],int table_cnt, int refrence_string[REFERENCEMAX],int reference_cnt,int frame_pool[POOLMAX],int frame_cnt) {
    int current_timestamp = 1;
    int fault_count = 0;
    for (int i = 0; i < reference_cnt; i++) {
        current_timestamp++;
        int page_number = refrence_string[i];
        if (page_table[page_number].is_valid == 1) {
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count += 1;
        }
        else {
            fault_count++;
            if (frame_cnt != 0) {
            int free_frame_index;
            for (int i = 0; i < frame_cnt; i++) {
                if (frame_pool[i] != 0) {
                    free_frame_index = i;
                    break;
                }
            }
            page_table[page_number].arrival_timestamp = current_timestamp;
            page_table[page_number].frame_number = frame_pool[free_frame_index];
            page_table[page_number].is_valid = 1;
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count = 1;
            frame_pool[free_frame_index] = 0;
            (frame_cnt)--;
            }
            else {
                int smallest_reference_count = 0;
                for (int i = 0; i < table_cnt; i++) {
                    if (page_table[i].is_valid == 1) {
                        smallest_reference_count = i;
                        break;
                    }
                }  
                for (int i = smallest_reference_count + 1; i < table_cnt; i++) {
                    if (page_table[i].is_valid == 1 && page_table[i].reference_count <= page_table[smallest_reference_count].reference_count) {
                        smallest_reference_count = i;
                    }
                }
                page_table[smallest_reference_count].is_valid = 0;
                page_table[page_number].frame_number = page_table[smallest_reference_count].frame_number;
                page_table[smallest_reference_count].frame_number = -1;
                page_table[smallest_reference_count].arrival_timestamp = -1;
                page_table[smallest_reference_count].reference_count = -1;
                page_table[smallest_reference_count].last_access_timestamp = -1;
                page_table[page_number].arrival_timestamp = current_timestamp;
                page_table[page_number].is_valid = 1;
                page_table[page_number].last_access_timestamp = current_timestamp;
                page_table[page_number].reference_count = 1;

            }
        }   
    }
    return fault_count; 
}

