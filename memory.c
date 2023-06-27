#include "oslabs.h"

struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id)
{
    int best_fit_size = -1; 
    int best_fit_index = -1; 

    for (int i = 0; i < *map_cnt; i++) 
    {
        struct MEMORY_BLOCK current_block = memory_map[i];
        if (current_block.process_id == 0 && current_block.segment_size >= request_size) 
        {
            if (best_fit_size == -1 || current_block.segment_size < best_fit_size) 
            {
                best_fit_size = current_block.segment_size;
                best_fit_index = i;
            }
        }
    }

    struct MEMORY_BLOCK null_block = {0, 0, 0, 0};
    if (best_fit_index == -1) return null_block;

    struct MEMORY_BLOCK best_fit_block = memory_map[best_fit_index];
    if (best_fit_block.segment_size == request_size) 
    {
        best_fit_block.process_id = process_id;
        memory_map[best_fit_index] = best_fit_block;
        return best_fit_block;
    } 
    else 
    {
        best_fit_block.process_id = process_id;
        best_fit_block.end_address = best_fit_block.start_address + request_size - 1;
        best_fit_block.segment_size = request_size;

        struct MEMORY_BLOCK new_free_block;
        new_free_block.start_address = best_fit_block.end_address + 1;
        new_free_block.end_address = memory_map[best_fit_index].end_address;
        new_free_block.segment_size = memory_map[best_fit_index].segment_size - request_size;
        new_free_block.process_id = 0;

        memory_map[best_fit_index] = best_fit_block;
        memory_map[*map_cnt] = new_free_block;
        (*map_cnt)++;

        return best_fit_block;
    }
}
