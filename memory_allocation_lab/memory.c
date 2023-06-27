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

struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt, int process_id) 
{
    int first_fit_index = -1;

    for (int i = 0; i < *map_cnt; i++) 
    {
        struct MEMORY_BLOCK current_block = memory_map[i];
        if (current_block.process_id == 0 && current_block.segment_size >= request_size) 
        {
            first_fit_index = i;
            break;
        }
    }

    struct MEMORY_BLOCK null_block = {0, 0, 0, 0};
    if (first_fit_index == -1) return null_block;

    struct MEMORY_BLOCK first_fit_block = memory_map[first_fit_index];
    if (first_fit_block.segment_size == request_size) 
    {
        first_fit_block.process_id = process_id;
        memory_map[first_fit_index] = first_fit_block;
        return first_fit_block;
    } 
    else 
    {
        first_fit_block.process_id = process_id;
        first_fit_block.end_address = first_fit_block.start_address + request_size - 1;
        first_fit_block.segment_size = request_size;

        struct MEMORY_BLOCK new_free_block;
        new_free_block.start_address = first_fit_block.end_address + 1;
        new_free_block.end_address = memory_map[first_fit_index].end_address;
        new_free_block.segment_size = memory_map[first_fit_index].segment_size - request_size;
        new_free_block.process_id = 0;

        memory_map[first_fit_index] = first_fit_block;
        memory_map[*map_cnt] = new_free_block;
        (*map_cnt)++;

        return first_fit_block;
    }
}

struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id)
{
    int worst_fit_size = -1;
    int worst_fit_index = -1;

    for (int i = 0; i < *map_cnt; i++) 
    {
        struct MEMORY_BLOCK current_block = memory_map[i];
        if (current_block.process_id == 0 && current_block.segment_size >= request_size)
        {
            if (current_block.segment_size > worst_fit_size) 
            {
                worst_fit_size = current_block.segment_size;
                worst_fit_index = i;
            }
        }
    }

    struct MEMORY_BLOCK null_block = {0, 0, 0, 0};
    if (worst_fit_index == -1) return null_block;

    struct MEMORY_BLOCK worst_fit_block = memory_map[worst_fit_index];
    if (worst_fit_block.segment_size == request_size) 
    {
        worst_fit_block.process_id = process_id;
        memory_map[worst_fit_index] = worst_fit_block;
        return worst_fit_block;
    } 
    else 
    {
        worst_fit_block.process_id = process_id;
        worst_fit_block.end_address = worst_fit_block.start_address + request_size - 1;
        worst_fit_block.segment_size = request_size;

        struct MEMORY_BLOCK new_free_block;
        new_free_block.start_address = worst_fit_block.end_address + 1;
        new_free_block.end_address = memory_map[worst_fit_index].end_address;
        new_free_block.segment_size = memory_map[worst_fit_index].segment_size - request_size;
        new_free_block.process_id = 0;

        memory_map[worst_fit_index] = worst_fit_block;
        memory_map[*map_cnt] = new_free_block;
        (*map_cnt)++;

        return worst_fit_block;
    }
}

struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address)
{
    int next_fit_index = -1;

    for (int i = 0; i < *map_cnt; i++) 
    {
        struct MEMORY_BLOCK current_block = memory_map[i];
        if (current_block.process_id == 0 && current_block.segment_size >= request_size && current_block.start_address >= last_address) 
        {
            next_fit_index = i;
            break;
        }
    }

    //If no block found starting from last_address, search from the beginning
    if (next_fit_index == -1) 
    {
        for (int i = 0; i < *map_cnt; i++) 
        {
            struct MEMORY_BLOCK current_block = memory_map[i];
            if (current_block.process_id == 0 && current_block.segment_size >= request_size) 
            {
                next_fit_index = i;
                break;
            }
        }
    }

    struct MEMORY_BLOCK null_block = {0, 0, 0, 0};
    if (next_fit_index == -1) return null_block;

    struct MEMORY_BLOCK next_fit_block = memory_map[next_fit_index];
    if (next_fit_block.segment_size == request_size) 
    {
        next_fit_block.process_id = process_id;
        memory_map[next_fit_index] = next_fit_block;
        return next_fit_block;
    } 
    else 
    {
        next_fit_block.process_id = process_id;
        next_fit_block.end_address = next_fit_block.start_address + request_size - 1;
        next_fit_block.segment_size = request_size;

        struct MEMORY_BLOCK new_free_block;
        new_free_block.start_address = next_fit_block.end_address + 1;
        new_free_block.end_address = memory_map[next_fit_index].end_address;
        new_free_block.segment_size = memory_map[next_fit_index].segment_size - request_size;
        new_free_block.process_id = 0;

        memory_map[next_fit_index] = next_fit_block;
        memory_map[*map_cnt] = new_free_block;
        (*map_cnt)++;

        return next_fit_block;
    }
}

void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt)
{
    int freed_index = -1;
    
    for(int i=0; i < *map_cnt; i++)
    {
        if (memory_map[i].start_address == freed_block.start_address && memory_map[i].end_address == freed_block.end_address)
        {
            freed_index = i;
            memory_map[i].process_id = 0;
            break;
        }
    }
    
    if (freed_index == -1) return; // freed_block is not found
    
    // Merge with previous block if it's free
    if (freed_index > 0 && memory_map[freed_index - 1].process_id == 0)
    {
        memory_map[freed_index - 1].end_address = memory_map[freed_index].end_address;
        memory_map[freed_index - 1].segment_size += memory_map[freed_index].segment_size;
        for(int i= freed_index; i < *map_cnt - 1; i++) 
        {
            memory_map[i] = memory_map[i + 1];
        }
        (*map_cnt)--;
        freed_index--;
    }
    
    // Merge with next block if it's free
    if (freed_index < *map_cnt - 1 && memory_map[freed_index + 1].process_id == 0)
    {
        memory_map[freed_index].end_address = memory_map[freed_index + 1].end_address;
        memory_map[freed_index].segment_size += memory_map[freed_index + 1].segment_size;
        for(int i= freed_index + 1; i < *map_cnt - 1; i++) 
        {
            memory_map[i] = memory_map[i + 1];
        }
        (*map_cnt)--;
    }
}

