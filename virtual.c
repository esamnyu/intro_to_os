#include "oslabs.h"
#include <limits.h>
#include <stdio.h>
#include <assert.h>

int process_page_access_fifo(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    // Check if page is in memory
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }

    // If not, check if there are any free frames
    if (*frame_cnt > 0) {
        int frame = frame_pool[--(*frame_cnt)];  // Get and remove frame from frame pool
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        printf("Page fault (free frame used): page = %d, frame = %d, timestamp = %d\n", page_number, frame, current_timestamp);
        return frame;
    }

    // If not, perform page replacement using FIFO
    int oldest_page = 0;
    for (int i = 1; i < *table_cnt; i++) {
        if (page_table[i].is_valid && (page_table[i].arrival_timestamp < page_table[oldest_page].arrival_timestamp)) {
            oldest_page = i;
        }
    }

    int frame = page_table[oldest_page].frame_number;
    page_table[oldest_page].is_valid = 0;
    page_table[oldest_page].frame_number = -1;
    page_table[oldest_page].arrival_timestamp = -1;
    page_table[oldest_page].last_access_timestamp = -1;
    page_table[oldest_page].reference_count = -1;

    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;
    printf("Page fault (page replaced): page = %d, frame = %d, timestamp = %d\n", page_number, frame, current_timestamp);

    return frame;
}


int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int faults = 0;
    int timestamp = 0;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        timestamp++;

        if (page_table[page_number].is_valid) {
            page_table[page_number].last_access_timestamp = timestamp;
            page_table[page_number].reference_count++;
        } else {
            faults++;

            if (frame_cnt > 0) {
                int frame = frame_pool[--frame_cnt];

                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = frame;
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;
            } else {
                int oldest_page_number = 0;
                for (int j = 1; j < table_cnt; j++) {
                    if (page_table[j].is_valid && page_table[j].arrival_timestamp < page_table[oldest_page_number].arrival_timestamp) {
                        oldest_page_number = j;
                    }
                }

                int frame = page_table[oldest_page_number].frame_number;

                page_table[oldest_page_number].is_valid = 0;
                page_table[oldest_page_number].frame_number = -1;
                page_table[oldest_page_number].arrival_timestamp = -1;
                page_table[oldest_page_number].last_access_timestamp = -1;
                page_table[oldest_page_number].reference_count = 0;

                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = frame;
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;
            }
        }
    }

    return faults;
}

int process_page_access_lfu(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp){
    
    // If page already in memory
    if(page_table[page_number].is_valid){
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }

    // If there are any free frames
    if(*frame_cnt > 0){
        --(*frame_cnt);
        int free_frame = frame_pool[*frame_cnt];
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = free_frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        return free_frame;
    }

    // Page replacement
    int min_count = INT_MAX;
    int oldest_timestamp = INT_MAX;
    int replace_index = -1;
    
    // Find the least frequently used page
    for(int i = 0; i < *table_cnt; ++i){
        if(page_table[i].is_valid && page_table[i].reference_count <= min_count){
            if(page_table[i].reference_count < min_count || (page_table[i].reference_count == min_count && page_table[i].arrival_timestamp < oldest_timestamp)){
                min_count = page_table[i].reference_count;
                oldest_timestamp = page_table[i].arrival_timestamp;
                replace_index = i;
            }
        }
    }
    
    if(replace_index == -1){
        // Couldn't find a page to replace
        return -1;
    }

    // Replace the least frequently used page
    int replace_frame = page_table[replace_index].frame_number;
    page_table[replace_index].is_valid = 0;
    page_table[replace_index].frame_number = -1;
    page_table[replace_index].arrival_timestamp = 0;
    page_table[replace_index].last_access_timestamp = 0;
    page_table[replace_index].reference_count = 0;

    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = replace_frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;

    return replace_frame;
}

int process_page_access_lru(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    } else {
        if (*frame_cnt > 0) {
            int frame = frame_pool[--(*frame_cnt)];

            page_table[page_number].is_valid = 1;
            page_table[page_number].frame_number = frame;
            page_table[page_number].arrival_timestamp = current_timestamp;
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count = 1;

            return frame;
        } else {
            int lru_page = 0;
            for (int i = 1; i < *table_cnt; i++) {
                if (page_table[i].is_valid && (page_table[i].last_access_timestamp < page_table[lru_page].last_access_timestamp)) {
                    lru_page = i;
                }
            }

            int frame = page_table[lru_page].frame_number;

            page_table[lru_page].is_valid = 0;
            page_table[lru_page].frame_number = -1;
            page_table[lru_page].arrival_timestamp = -1;
            page_table[lru_page].last_access_timestamp = -1;
            page_table[lru_page].reference_count = 0;

            page_table[page_number].is_valid = 1;
            page_table[page_number].frame_number = frame;
            page_table[page_number].arrival_timestamp = current_timestamp;
            page_table[page_number].last_access_timestamp = current_timestamp;
            page_table[page_number].reference_count = 1;

            return frame;
        }
    }
}

int count_page_faults_lru(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int faults = 0;
    int timestamp = 1;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        if (page_table[page_number].is_valid) {
            page_table[page_number].last_access_timestamp = timestamp;
            page_table[page_number].reference_count++;
        } else {
            if (frame_cnt > 0) {
                int frame = frame_pool[--frame_cnt];

                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = frame;
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;
                
                faults++;
            } else {
                int lru_page = 0;
                for (int j = 1; j < table_cnt; j++) {
                    if (page_table[j].is_valid && page_table[j].last_access_timestamp < page_table[lru_page].last_access_timestamp) {
                        lru_page = j;
                    }
                }

                int frame = page_table[lru_page].frame_number;

                page_table[lru_page].is_valid = 0;
                page_table[lru_page].frame_number = -1;
                page_table[lru_page].arrival_timestamp = 0;
                page_table[lru_page].last_access_timestamp = 0;
                page_table[lru_page].reference_count = 0;

                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = frame;
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;

                faults++;
            }
        }
        timestamp++;
    }

    return faults;
}

int count_page_faults_lfu(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int faults = 0;
    int timestamp = 1;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        if (page_table[page_number].is_valid) {
            page_table[page_number].last_access_timestamp = timestamp;
            page_table[page_number].reference_count++;
        } else {
            if (frame_cnt > 0) {
                int frame = frame_pool[--frame_cnt];

                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = frame;
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;
                
                faults++;
            } else {
                int lfu_page = -1;
                int min_reference_count = INT_MAX;
                int earliest_arrival_timestamp = INT_MAX;

                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid) {
                        if (page_table[j].reference_count < min_reference_count) {
                            min_reference_count = page_table[j].reference_count;
                            earliest_arrival_timestamp = page_table[j].arrival_timestamp;
                            lfu_page = j;
                        } else if (page_table[j].reference_count == min_reference_count && page_table[j].arrival_timestamp < earliest_arrival_timestamp) {
                            earliest_arrival_timestamp = page_table[j].arrival_timestamp;
                            lfu_page = j;
                        }
                    }
                }

                int frame = page_table[lfu_page].frame_number;

                page_table[lfu_page].is_valid = 0;
                page_table[lfu_page].frame_number = -1;
                page_table[lfu_page].arrival_timestamp = 0;
                page_table[lfu_page].last_access_timestamp = 0;
                page_table[lfu_page].reference_count = 0;

                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = frame;
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;

                faults++;
            }
        }
        timestamp++;
    }

    return faults;
}

// int main() {
//     // Initialize page table
//     struct PTE page_table[TABLEMAX];
//     int table_cnt = TABLEMAX;

//     // Assume all pages are initially not in memory
//     for (int i = 0; i < table_cnt; i++) {
//         page_table[i].is_valid = 0;
//         page_table[i].frame_number = -1;
//         page_table[i].arrival_timestamp = -1;
//         page_table[i].last_access_timestamp = -1;
//         page_table[i].reference_count = -1;
//     }

//     // Initialize frame pool with some frames
//     int frame_pool[POOLMAX] = {10, 20, 30, 40};
//     int frame_cnt = 4;

//     // Reference string and count
//     int reference_string[REFERENCEMAX] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
//     int reference_cnt = 12;

//     // Run count_page_faults_fifo
//     int faults = count_page_faults_fifo(page_table, table_cnt, reference_string, reference_cnt, frame_pool, frame_cnt);

//     // Check the result
//     printf("Page faults: %d\n", faults);
//     assert(faults == 8);

//     return 0;
// }
// // The assert statement will abort the program if the number of faults is not 8, printing an error message similar to the one you're seeing.

// // Please replace the reference_string and reference_cnt with the actual sequence of page accesses and length used in your test case, as I don't know what specific sequence causes the discrepancy between the expected and actual number of page faults.

// // Run the test case and see if it passes. If it does not pass, use print statements or a debugger to find out what's happening inside count_page_faults_fifo and process_page_access_fifo.






