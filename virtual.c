#include "oslabs.h"
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

// #define QUEUEMAX 10
// #define MAPMAX 10
// #define TABLEMAX 10
// #define POOLMAX 10
// #define REFERENCEMAX 20

// struct PTE {
//     int is_valid;
//     int frame_number;
//     int arrival_timestamp;
//     int last_access_timestamp;
//     int reference_count;
// };

int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;
    int current_timestamp = 0;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        
        // Save the old validity of the page
        int old_validity = page_table[page_number].is_valid;
        
        // Process the page access
        process_page_access_fifo(page_table, &table_cnt, page_number, frame_pool, &frame_cnt, current_timestamp);
        
        // If the page was invalid before and is valid now, it's a page fault
        if (!old_validity && page_table[page_number].is_valid) {
            page_faults++;
        }

        // Increment the current timestamp for every page access
        current_timestamp++;
    }

    return page_faults;
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

int process_page_access_fifo(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp){
    // Check if the page being referenced is already in memory
    if (page_table[page_number].is_valid == 1){
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count++;
        return page_table[page_number].frame_number;
    }

    // Check if there are any free frames
    if (*frame_cnt > 0){
        int frame_number = frame_pool[0];  // remove the first frame from the pool
        for (int i = 0; i < (*frame_cnt - 1); i++) {
            frame_pool[i] = frame_pool[i + 1]; // Shift the remaining elements to left
        }
        (*frame_cnt)--;  // decrease the frame count by 1
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = frame_number;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        return frame_number;
    }

    // If no free frames, need to replace a page
    int smallest_arrival_timestamp = INT_MAX;
    int frame_to_replace = -1;
    for (int i = 0; i < *table_cnt; i++){
        if (page_table[i].is_valid == 1 && page_table[i].arrival_timestamp < smallest_arrival_timestamp){
            smallest_arrival_timestamp = page_table[i].arrival_timestamp;
            frame_to_replace = i;
        }
    }

    // Invalidate the selected page and reset its values
    int frame_number = page_table[frame_to_replace].frame_number;
    page_table[frame_to_replace].is_valid = 0;
    page_table[frame_to_replace].frame_number = -1;
    page_table[frame_to_replace].arrival_timestamp = -1;
    page_table[frame_to_replace].last_access_timestamp = -1;
    page_table[frame_to_replace].reference_count = -1;

    // Update the page-table entry of the newly-referenced page
    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = frame_number;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;
    page_table[page_number].reference_count = 1;

    return frame_number;
}


int process_page_access_lru(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp) {
    // This is just a stub function. Replace with real implementation later.
    return 0;
}

int count_page_faults_lru(struct PTE page_table[TABLEMAX],int table_cnt, int reference_string[REFERENCEMAX],int reference_cnt,int frame_pool[POOLMAX],int frame_cnt) {
    int faults = 0;
    int timestamp = 1;
    for (int i = 0; i < reference_cnt; ++i) {
        int page_number = reference_string[i];
        if (page_table[page_number].is_valid) {
            // Page is already in memory, update last_access_timestamp
            page_table[page_number].last_access_timestamp = timestamp;
        } else {
            if (frame_cnt > 0) {
                // There are free frames, use one
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = frame_pool[--frame_cnt];
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;
            } else {
                // No free frames, apply LRU policy to replace a page
                int lru_page = -1;
                int earliest_timestamp = INT_MAX;
                for (int j = 0; j < table_cnt; ++j) {
                    if (page_table[j].is_valid && page_table[j].last_access_timestamp < earliest_timestamp) {
                        lru_page = j;
                        earliest_timestamp = page_table[j].last_access_timestamp;
                    }
                }

                // Invalidate the LRU page
                page_table[lru_page].is_valid = 0;
                page_table[lru_page].arrival_timestamp = 0;
                page_table[lru_page].last_access_timestamp = 0;
                page_table[lru_page].reference_count = 0;

                // Use the freed frame for the new page
                page_table[page_number].is_valid = 1;
                page_table[page_number].frame_number = page_table[lru_page].frame_number;
                page_table[page_number].arrival_timestamp = timestamp;
                page_table[page_number].last_access_timestamp = timestamp;
                page_table[page_number].reference_count = 1;
            }
            ++faults;
        }
        ++timestamp;
    }
    return faults;
}


int process_page_access_lfu(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp) {
    // This is just a stub function. Replace with real implementation later.
    return 0;
}




int main() {
    struct PTE page_table[TABLEMAX];
    int table_cnt = 5;
    int frame_pool[POOLMAX];
    int frame_cnt = 3;

    // Initialize the page table and frame pool
    for (int i = 0; i < table_cnt; i++) {
        page_table[i].is_valid = 0;
        page_table[i].frame_number = -1;
        page_table[i].arrival_timestamp = -1;
        page_table[i].last_access_timestamp = -1;
        page_table[i].reference_count = -1;
    }
    for (int i = 0; i < frame_cnt; i++) {
        frame_pool[i] = i;
    }

    int current_timestamp = 1;
    int expected_faults = 2;
    int faults = 0;

    for (int i = 0; i < table_cnt; i++) {
        int frame = process_page_access_fifo(page_table, &table_cnt, i, frame_pool, &frame_cnt, current_timestamp);
        if (frame == -1) {
            faults++;
        }
        current_timestamp++;
    }

    printf("Expected faults: %d, Actual faults: %d\n", expected_faults, faults);
    return 0;
}
