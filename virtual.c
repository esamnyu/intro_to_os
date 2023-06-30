#include "oslabs.h"
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int page_faults = 0;
    int frame_index = 0;

    for (int i = 0; i < reference_cnt; i++) {
        int found = 0;
        for (int j = 0; j < table_cnt; j++) {
            if (page_table[j].is_valid && page_table[j].frame_number == reference_string[i]) {
                found = 1;
                break;
            }
        }

        if (!found) {
            page_faults++;
            if (frame_index < frame_cnt) {
                // Still have free frames in the pool
                page_table[frame_index].is_valid = 1;
                page_table[frame_index].frame_number = reference_string[i];
                frame_index++;
            } else {
                // Need to replace the oldest frame (FIFO)
                int oldest_index = 0;
                for (int j = 1; j < table_cnt; j++) {
                    if (page_table[j].arrival_timestamp < page_table[oldest_index].arrival_timestamp) {
                        oldest_index = j;
                    }
                }
                page_table[oldest_index].is_valid = 1;
                page_table[oldest_index].frame_number = reference_string[i];
                page_table[oldest_index].arrival_timestamp = i;  // Assuming the reference string index can serve as the timestamp
            }
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

int count_page_faults_lru(struct PTE page_table[TABLEMAX],int table_cnt, int refrence_string[REFERENCEMAX],int reference_cnt,int frame_pool[POOLMAX],int frame_cnt) {
    // This is just a stub function. Replace with real implementation later.
    return 0;
}

int process_page_access_lfu(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp) {
    // This is just a stub function. Replace with real implementation later.
    return 0;
}


// int main() {
//     // Test 1
//     {
//         struct PTE page_table[TABLEMAX] = {0};
//         int frame_pool[POOLMAX] = {0};
//         int frame_cnt = 1; 
//         int table_cnt = 1; 
//         int reference_string[REFERENCEMAX] = {1, 1, 1, 1, 1, 1, 1, 1};

//         int faults = count_page_faults_fifo(page_table, table_cnt, reference_string, 8, frame_pool, frame_cnt);
//         printf("Faults for Test 1: %d\n", faults);
//         assert(faults == 0);
//     }

//     // Test 2
//     {
//         struct PTE page_table[TABLEMAX] = {0};
//         int frame_pool[POOLMAX] = {0};
//         int frame_cnt = 1; 
//         int table_cnt = 8; 
//         int reference_string[REFERENCEMAX] = {1, 2, 3, 4, 5, 6, 7, 8};

//         int faults = count_page_faults_fifo(page_table, table_cnt, reference_string, 8, frame_pool, frame_cnt);
//         assert(faults == 7);
//     }

//     // Test 3
//     {
//         struct PTE page_table[TABLEMAX] = {0};
//         int frame_pool[POOLMAX] = {0};
//         int frame_cnt = 2; 
//         int table_cnt = 8; 
//         int reference_string[REFERENCEMAX] = {1, 2, 3, 4, 5, 6, 7, 8};

//         int faults = count_page_faults_fifo(page_table, table_cnt, reference_string, 8, frame_pool, frame_cnt);
//         assert(faults == 6);
//     }

//     printf("All tests passed!\n");

//     return 0;
// }







