#include "oslabs.h"
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

// New helper function
bool process_page_and_check_fault(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    // Previous frame count before processing the page
    int prev_frame_count = *frame_cnt;

    // Process the page access
    process_page_access_fifo(page_table, table_cnt, page_number, frame_pool, frame_cnt, current_timestamp);

    // If frame count decreased, it means a page fault occurred
    if (*frame_cnt < prev_frame_count) {
        return true;
    }

    return false;
}

int process_page_access_fifo(struct PTE page_table[TABLEMAX], int *table_cnt, int page_number, int frame_pool[POOLMAX], int *frame_cnt, int current_timestamp) {
    // Check if page is already in memory
    if (page_table[page_number].is_valid) {
        page_table[page_number].last_access_timestamp = current_timestamp;
        return page_table[page_number].frame_number;
    }

    // Check if there are free frames available
    if (*frame_cnt > 0) {
        int frame = frame_pool[--(*frame_cnt)];

        // Update the page table
        page_table[page_number].is_valid = 1;
        page_table[page_number].frame_number = frame;
        page_table[page_number].arrival_timestamp = current_timestamp;
        page_table[page_number].last_access_timestamp = current_timestamp;

        return -1; // Indicate a page fault occurred as the page was not in memory
    }

    // Find the oldest page for replacement (FIFO)
    int oldest_page = 0;
    for (int i = 0; i < TABLEMAX; i++) {
        if (page_table[i].arrival_timestamp < page_table[oldest_page].arrival_timestamp) {
            oldest_page = i;
        }
    }

    // Replace the oldest page
    int frame = page_table[oldest_page].frame_number;
    page_table[oldest_page].is_valid = 0;
    page_table[oldest_page].frame_number = -1;
    page_table[oldest_page].arrival_timestamp = 0;
    page_table[oldest_page].last_access_timestamp = 0;

    // Add the new page
    page_table[page_number].is_valid = 1;
    page_table[page_number].frame_number = frame;
    page_table[page_number].arrival_timestamp = current_timestamp;
    page_table[page_number].last_access_timestamp = current_timestamp;

    return -1; // Indicate a page fault occurred as a page replacement was needed
}

int count_page_faults_fifo(struct PTE page_table[TABLEMAX], int table_cnt, int reference_string[REFERENCEMAX], int reference_cnt, int frame_pool[POOLMAX], int frame_cnt) {
    int faults = 0;
    int current_timestamp = 1;

    for (int i = 0; i < reference_cnt; i++) {
        int page_number = reference_string[i];

        bool page_fault_occurred = process_page_and_check_fault(page_table, &table_cnt, page_number, frame_pool, &frame_cnt, current_timestamp);

        if (page_fault_occurred) {
            faults++;
        }

        current_timestamp++;
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
        struct PTE* page = &page_table[page_number];

        // Page is in memory
        if (page->valid_bit) {
            page->last_access_timestamp = timestamp;
            page->reference_count++;
        } else {
            // Page fault
            faults++;

            // There are free frames
            if (frame_cnt > 0) {
                page->frame_number = frame_pool[--frame_cnt];
                page->valid_bit = true;
                page->arrival_timestamp = timestamp;
                page->last_access_timestamp = timestamp;
                page->reference_count = 1;
            } else {
                // No free frames, need to replace a page
                int lru_page_number = -1;
                int lru_timestamp = INT_MAX;

                // Find the least recently used page
                for (int j = 0; j < table_cnt; j++) {
                    if (page_table[j].valid_bit && page_table[j].last_access_timestamp < lru_timestamp) {
                        lru_timestamp = page_table[j].last_access_timestamp;
                        lru_page_number = j;
                    }
                }

                // Replace the least recently used page
                struct PTE* lru_page = &page_table[lru_page_number];
                lru_page->valid_bit = false;
                lru_page->arrival_timestamp = 0;
                lru_page->last_access_timestamp = 0;
                lru_page->reference_count = 0;

                // Assign the newly freed frame to the current page
                page->frame_number = lru_page->frame_number;
                lru_page->frame_number = -1;
                page->valid_bit = true;
                page->arrival_timestamp = timestamp;
                page->last_access_timestamp = timestamp;
                page->reference_count = 1;
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

int main() {
    struct PTE page_table[TABLEMAX];
    int frame_pool[POOLMAX] = {0, 1, 2, 3, 4}; // Assume frame pool size is 5
    int table_cnt = 0;
    int frame_cnt = POOLMAX;
    int current_timestamp = 0;
    
    // Initialize the page table
    for (int i = 0; i < TABLEMAX; i++) {
        page_table[i].is_valid = 0;
        page_table[i].frame_number = -1;
        page_table[i].arrival_timestamp = 0;
        page_table[i].last_access_timestamp = 0;
    }

    // Access pages in this order: 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4
    int pages_to_access[] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4};
    int number_of_page_faults = 0;
    for (int i = 0; i < 13; i++) {
        int frame = process_page_access_fifo(page_table, &table_cnt, pages_to_access[i], frame_pool, &frame_cnt, current_timestamp++);
        if (frame < 0) {
            number_of_page_faults++;
        }
    }

    // Print the number of page faults
    printf("Number of page faults: %d\n", number_of_page_faults);

    // The expected number of page faults is 8, because we are using FIFO and the frame pool size is 5.
    // The first 5 accesses will cause page faults because the pages are not in memory.
    // The next 3 accesses (pages 5, 6, 7) will also cause page faults because they are not in memory and will replace the oldest pages (pages 0, 1, 2).
    // The final 5 accesses (pages 0, 1, 2, 3, 4) will not cause page faults because these pages are now in memory.
    assert(number_of_page_faults == 8);

    return 0;
}





