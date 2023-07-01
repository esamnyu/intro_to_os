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
    int frame_index = 0;

    for (int i = 0; i < reference_cnt; i++) {
        bool found = false;
        for (int j = 0; j < table_cnt; j++) {
            if (page_table[j].is_valid && page_table[j].frame_number == reference_string[i]) {
                found = true;
                page_table[j].last_access_timestamp = i;
                page_table[j].reference_count++;
                break;
            }
        }

        if (!found) {
            page_faults++;
            if (frame_index < frame_cnt) {
                // Still have free frames in the pool
                page_table[frame_index].is_valid = true;
                page_table[frame_index].frame_number = reference_string[i];
                page_table[frame_index].arrival_timestamp = i;
                page_table[frame_index].last_access_timestamp = i;
                page_table[frame_index].reference_count = 1;
                frame_index++;
            } else {
                // Need to replace the oldest frame (FIFO)
                int oldest_index = 0;
                for (int j = 1; j < table_cnt; j++) {
                    if (page_table[j].arrival_timestamp < page_table[oldest_index].arrival_timestamp) {
                        oldest_index = j;
                    }
                }
                page_table[oldest_index].is_valid = true;
                page_table[oldest_index].frame_number = reference_string[i];
                page_table[oldest_index].arrival_timestamp = i;
                page_table[oldest_index].last_access_timestamp = i;
                page_table[oldest_index].reference_count = 1;
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


void print_results(int result, int expected) {
    if(result == expected) {
        printf("Test Passed: Received %d as expected.\n", result);
    } else {
        printf("Test Failed: Expected %d but received %d.\n", expected, result);
    }
}

int main() {
    // Test Case 1: Expected page faults = 10
    struct PTE page_table1[TABLEMAX] = {0}; // Initialize all elements to 0/false
    int table_cnt1 = 10;
    int reference_string1[REFERENCEMAX] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int reference_cnt1 = 10;
    int frame_pool1[POOLMAX] = {0, 1, 2, 3, 4};
    int frame_cnt1 = 5; // Less frames than pages

    int result1 = count_page_faults_fifo(page_table1, table_cnt1, reference_string1, reference_cnt1, frame_pool1, frame_cnt1);
    print_results(result1, 10); // We expect 10 page faults here

    // Test Case 2: Expected page faults = 9
    struct PTE page_table2[TABLEMAX] = {0}; // Initialize all elements to 0/false
    int table_cnt2 = 10;
    int reference_string2[REFERENCEMAX] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    int reference_cnt2 = 9;
    int frame_pool2[POOLMAX] = {0, 1, 2, 3, 4};
    int frame_cnt2 = 5; // Less frames than pages

    int result2 = count_page_faults_fifo(page_table2, table_cnt2, reference_string2, reference_cnt2, frame_pool2, frame_cnt2);
    print_results(result2, 9); // We expect 9 page faults here

    return 0;
}

