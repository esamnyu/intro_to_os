
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

int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], 
                           int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int faults = 0;
    int timestamp = 0;  // Initialize timestamp to 0

    // Process each page access in the reference string
    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        
        timestamp++;  // Increment timestamp before calling process_page_access_fifo

        int frame_number = process_page_access_fifo(page_table, &table_cnt, page_number, frame_pool, &frame_cnt, timestamp);

        // If the frame number returned is -1, then a page fault has occurred
        if (frame_number == -1) {
            faults++;
        }
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
        return frame_number;  // return frame_number not -1
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

    return frame_number;  // return frame_number not -1
}





int process_page_access_lru(struct PTE page_table[TABLEMAX],int *table_cnt, int page_number, int frame_pool[POOLMAX],int *frame_cnt, int current_timestamp){
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
        return frame_number;  // return frame_number not -1
    }

    // If no free frames, need to replace a page
    int smallest_last_access_timestamp = INT_MAX;
    int frame_to_replace = -1;
    for (int i = 0; i < *table_cnt; i++){
        if (page_table[i].is_valid == 1 && page_table[i].last_access_timestamp < smallest_last_access_timestamp){
            smallest_last_access_timestamp = page_table[i].last_access_timestamp;
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

    return frame_number;  // return frame_number not -1
}



int count_page_faults_lru(struct PTE page_table[TABLEMAX],int table_cnt, int reference_string[REFERENCEMAX],int reference_cnt,int frame_pool[POOLMAX],int frame_cnt) {
    int faults = 0;

    // Process each page access in the reference string
    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];
        
        // Track the validity of the page before the function call
        int was_valid = page_table[page_number].is_valid;
        
        // Call the function
        int frame_number = process_page_access_lru(page_table, &table_cnt, page_number, frame_pool, &frame_cnt, i + 1);
        
        // If the page was invalid before but is valid now, a page fault has occurred
        if (!was_valid && page_table[page_number].is_valid) {
            faults++;
        }
    }

    return faults;
}






int process_page_access_lfu(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count += 1;
        return page_table[page_number].frame_number;
    } else {
        int frame_number;
        if (*frame_cnt > 0) {
            frame_number = frame_pool[--(*frame_cnt)];
        } else {
            int min_ref_count = INT_MAX;
            int min_timestamp = INT_MAX;
            int replace_page_number = -1;
            for (int i = 0; i < *table_cnt; ++i) {
                if (page_table[i].is_valid && (page_table[i].reference_count < min_ref_count ||
                    (page_table[i].reference_count == min_ref_count && page_table[i].arrival_timestamp < min_timestamp))) {
                    min_ref_count = page_table[i].reference_count;
                    min_timestamp = page_table[i].arrival_timestamp;
                    replace_page_number = i;
                }
            }

            if (replace_page_number == -1) {
                printf("Error: No page to replace\n");
                return -1; // Error handling
            }

            frame_number = page_table[replace_page_number].frame_number;
            page_table[replace_page_number].is_valid = false;
            page_table[replace_page_number].frame_number = -1;
            page_table[replace_page_number].arrival_timestamp = -1;
            page_table[replace_page_number].last_access_timestamp = -1;
            page_table[replace_page_number].reference_count = -1;
        }

        page_table[page_number].is_valid = true;
        page_table[page_number].frame_number = frame_number;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;
        page_table[page_number].reference_count = 1;
        return frame_number;
    }
}
// void test_case_1() {
//     struct PTE page_table[TABLEMAX] = { {1, 0, 0, 0, 0}, {1, 1, 0, 0, 0}, {1, 2, 0, 0, 0} };
//     int table_cnt = 3;

//     int reference_string[REFERENCEMAX] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
//     int reference_cnt = 9;

//     int frame_pool[POOLMAX] = {};
//     int frame_cnt = 0;

//     int faults = count_page_faults_lru(page_table, table_cnt, reference_string, reference_cnt, frame_pool, frame_cnt);
//     printf("Test Case 1 - Page Faults: %d\n", faults);  // should print "Page Faults: 0"
// }

// void test_case_2() {
//     struct PTE page_table[TABLEMAX] = { {0, -1, 0, 0, 0}, {0, -1, 0, 0, 0}, {0, -1, 0, 0, 0} };
//     int table_cnt = 3;

//     int reference_string[REFERENCEMAX] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
//     int reference_cnt = 9;

//     int frame_pool[POOLMAX] = {0, 1, 2};
//     int frame_cnt = 3;

//     int faults = count_page_faults_lru(page_table, table_cnt, reference_string, reference_cnt, frame_pool, frame_cnt);
//     printf("Test Case 2 - Page Faults: %d\n", faults);  // should print "Page Faults: 3"
// }

// void test_case_3() {
//     struct PTE page_table[TABLEMAX] = { {1, 0, 0, 0, 0}, {0, -1, 0, 0, 0}, {0, -1, 0, 0, 0} };
//     int table_cnt = 3;

//     int reference_string[REFERENCEMAX] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
//     int reference_cnt = 9;

//     int frame_pool[POOLMAX] = {1, 2};
//     int frame_cnt = 2;

//     int faults = count_page_faults_lru(page_table, table_cnt, reference_string, reference_cnt, frame_pool, frame_cnt);
//     printf("Test Case 3 - Page Faults: %d\n", faults);  // should print "Page Faults: 2"
// }

// void test_case_4() {
//     struct PTE page_table[TABLEMAX] = { {0, -1, 0, 0, 0}, {0, -1, 0, 0, 0}, {0, -1, 0, 0, 0} };
//     int table_cnt = 3;

//     int reference_string[REFERENCEMAX] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
//     int reference_cnt = 9;

//     int frame_pool[POOLMAX] = {0, 1};
//     int frame_cnt = 2;

//     int faults = count_page_faults_lru(page_table, table_cnt, reference_string, reference_cnt, frame_pool, frame_cnt);
//     printf("Test Case 4 - Page Faults: %d\n", faults);  // should print "Page Faults: 6"
// }
void test_process_page_access_lru() {
    struct PTE page_table[TABLEMAX] = { {0, -1, 0, 0, 0}, {0, -1, 0, 0, 0}, {0, -1, 0, 0, 0} };
    int table_cnt = 3;
    int frame_pool[POOLMAX] = {30, 31, 32};
    int frame_cnt = 3;
    int page_number = 0;
    int current_timestamp = 1;

    int frame_number = process_page_access_lru(page_table, &table_cnt, page_number, frame_pool, &frame_cnt, current_timestamp);
    printf("Test Case - Returned Frame: %d\n", frame_number);  // should print "Returned Frame: 30"
}

// int main() {
//     test_process_page_access_lru();

//     return 0;
// }

