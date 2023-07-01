#include "oslabs.h"
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

int count_page_faults_fifo(struct PTE page_table[TABLEMAX],int table_cnt, int reference_string[REFERENCEMAX],int reference_cnt,int frame_pool[POOLMAX],int frame_cnt) {
    int page_faults = 0;
    int frame_index = 0;
    int timestamp = 1;
    
    for (int i = 0; i < reference_cnt; i++) {
        int page_found = 0;
        
        // Check if page is already in memory
        for (int j = 0; j < table_cnt; j++) {
            if (page_table[j].is_valid && page_table[j].frame_number == reference_string[i]) {
                page_table[j].last_access_timestamp = timestamp;
                page_table[j].reference_count += 1;
                page_found = 1;
                break;
            }
        }
        
        // Page is not in memory
        if (!page_found) {
            page_faults++;
            
            // If there are free frames, use one
            if (frame_index < frame_cnt) {
                int frame = frame_pool[frame_index++];
                page_table[reference_string[i]].is_valid = 1;
                page_table[reference_string[i]].frame_number = frame;
                page_table[reference_string[i]].arrival_timestamp = timestamp;
                page_table[reference_string[i]].last_access_timestamp = timestamp;
                page_table[reference_string[i]].reference_count = 1;
            } else {
                // Replace the oldest page
                int oldest_index = -1;
                int oldest_timestamp = INT_MAX;
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].is_valid && page_table[j].arrival_timestamp < oldest_timestamp) {
                        oldest_timestamp = page_table[j].arrival_timestamp;
                        oldest_index = j;
                    }
                }
                int replaced_frame = page_table[oldest_index].frame_number;
                page_table[oldest_index].is_valid = 0;
                page_table[oldest_index].arrival_timestamp = -1;
                page_table[oldest_index].last_access_timestamp = -1;
                page_table[oldest_index].reference_count = -1;
                
                page_table[reference_string[i]].is_valid = 1;
                page_table[reference_string[i]].frame_number = replaced_frame;
                page_table[reference_string[i]].arrival_timestamp = timestamp;
                page_table[reference_string[i]].last_access_timestamp = timestamp;
                page_table[reference_string[i]].reference_count = 1;
            }
        }
        
        timestamp++;
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

int process_page_access_fifo(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp) {
    // This is just a stub function. Replace with real implementation later.
    return 0;
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


// int main() {
//     struct PTE page_table[TABLEMAX];
//     int reference_string[REFERENCEMAX];
//     int frame_pool[POOLMAX];

//     // Populate the page_table, reference_string, and frame_pool with appropriate values

//     // For the purpose of the example, let's assume the table contains 5 entries, the reference string
//     // contains 10 entries, and the frame pool contains 3 frames.
//     int table_cnt = 5, reference_cnt = 10, frame_cnt = 3;

//     for(int i=0; i<table_cnt; i++) {
//         // Initialize all entries of page_table as invalid
//         page_table[i].is_valid = false;
//         page_table[i].frame_number = -1;
//         page_table[i].arrival_timestamp = -1;
//     }

//     // Initialize the reference_string, this should ideally be randomized or provided in the problem.
//     for(int i=0; i<reference_cnt; i++) {
//         reference_string[i] = i % table_cnt;
//     }

//     // Initialize the frame_pool, again this should be provided in the problem.
//     for(int i=0; i<frame_cnt; i++) {
//         frame_pool[i] = i;
//     }

//     int result = count_page_faults_fifo(page_table, table_cnt, reference_string, reference_cnt, frame_pool, frame_cnt);

//     // Expecting 10 faults, test will fail if the result is not 10
//     assert(result == 10);

//     printf("Test passed\n");

//     return 0;
// }

