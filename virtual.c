#include "oslabs.h"
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;

    for (int i = 0; i < reference_cnt; i++) {
        int page_num = reference_string[i];
        if (page_table[page_num].is_valid) {
            // Page is in memory, no page fault
            continue;
        }

        // Page fault
        page_faults++;

        if (frame_cnt > 0) {
            // We have free frames in the pool, allocate one to the page
            page_table[page_num].is_valid = 1;
            page_table[page_num].frame_number = frame_pool[0];
            page_table[page_num].arrival_timestamp = i;
            // Shift the frame_pool left, simulating removing a frame from the pool
            for (int j = 0; j < frame_cnt - 1; j++) {
                frame_pool[j] = frame_pool[j + 1];
            }
            frame_cnt--;
        } else {
            // No free frames, replace the oldest (FIFO)
            int oldest_page = -1;
            int oldest_frame = INT_MAX;
            for (int j = 0; j < table_cnt; j++) {
                if (page_table[j].is_valid && page_table[j].arrival_timestamp < oldest_frame) {
                    oldest_page = j;
                    oldest_frame = page_table[j].arrival_timestamp;
                }
            }
            // Invalidate the oldest page
            page_table[oldest_page].is_valid = 0;

            // Replace the frame from the oldest page with the new page
            page_table[page_num].is_valid = 1;
            page_table[page_num].frame_number = page_table[oldest_page].frame_number;
            page_table[page_num].arrival_timestamp = i;
        }
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
//     // Declare the required variables
//     struct PTE page_table[TABLEMAX];
//     int reference_string[REFERENCEMAX];
//     int frame_pool[POOLMAX];
//     int table_cnt = 0, frame_cnt = 0, reference_cnt = 0;

//     // Initialize frame pool
//     for (int i = 0; i < POOLMAX; i++) {
//         frame_pool[i] = -1;
//     }

//     // Initialize the page_table and reference_string to simulate your test scenario
//     // Below is a placeholder, you'll have to fill it with your actual data
//     for (int i = 0; i < TABLEMAX; i++) {
//         page_table[i].is_valid = 0; // starting all pages as invalid
//         page_table[i].frame_number = -1; // no frame is allocated initially
//         page_table[i].arrival_timestamp = 0; 
//         page_table[i].last_access_timestamp = 0;
//         page_table[i].reference_count = 0;
//     }

//     // Initialize the reference string such that it will generate 8 page faults with your settings.
//     // Again, below is a placeholder, fill it with actual data
//     reference_string[0] = 0;
//     reference_string[1] = 1;
//     reference_string[2] = 2;
//     reference_string[3] = 3;
//     reference_string[4] = 4;
//     reference_string[5] = 5;
//     reference_string[6] = 6;
//     reference_string[7] = 7;
//     reference_cnt = 8; // There are 8 pages in the reference string
//     frame_cnt = 3; // Let's say there are only 3 frames in the frame pool

//     int result = count_page_faults_fifo(page_table, TABLEMAX, reference_string, reference_cnt, frame_pool, frame_cnt);

//     printf("Expected Page Faults: 8, Actual Page Faults: %d\n", result);
    
//     return 0;
// }
