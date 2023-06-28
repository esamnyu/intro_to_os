#include "oslabs.h"
#include <limits.h>

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
