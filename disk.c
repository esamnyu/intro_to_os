#include "oslabs.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct RCB handle_request_arrival_fcfs(struct RCB request_queue[QUEUEMAX], int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp)
{
    if(current_request.request_id == 0) // if the current request is NULLRCB
    {
        return new_request;
    }
    else
    {
        request_queue[*queue_cnt] = new_request;
        (*queue_cnt)++;
        return current_request;
    }
}

struct RCB handle_request_completion_fcfs(struct RCB request_queue[QUEUEMAX], int *queue_cnt)
{
    if(*queue_cnt == 0) // if the request queue is empty
    {
        struct RCB NULLRCB = {0, 0, 0, 0, 0};
        return NULLRCB;
    }
    else
    {
        struct RCB next_request = request_queue[0];
        for(int i = 0; i < *queue_cnt - 1; i++) // shifting the queue to the left after removing the first element
        {
            request_queue[i] = request_queue[i + 1];
        }
        (*queue_cnt)--;
        return next_request;
    }
}

struct RCB handle_request_arrival_sstf(struct RCB request_queue[QUEUEMAX], int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp)
{
    if(current_request.request_id == 0) // if the disk is free
    {
        return new_request;
    }
    else
    {
        request_queue[*queue_cnt] = new_request; // adding the newly-arriving request to the request queue
        (*queue_cnt)++;
        return current_request;
    }
}

struct RCB handle_request_completion_sstf(struct RCB request_queue[QUEUEMAX], int *queue_cnt, int current_cylinder)
{
    if(*queue_cnt == 0) // if the request queue is empty
    {
        struct RCB NULLRCB = {0, 0, 0, 0, 0};
        return NULLRCB;
    }
    else
    {
        int min_distance = INT_MAX;
        int closest_index = 0;

        // find the request with the shortest seek time
        for(int i = 0; i < *queue_cnt; i++)
        {
            int distance = abs(request_queue[i].cylinder - current_cylinder);
            if(distance < min_distance || 
               (distance == min_distance && request_queue[i].arrival_timestamp < request_queue[closest_index].arrival_timestamp))
            {
                min_distance = distance;
                closest_index = i;
            }
        }

        struct RCB next_request = request_queue[closest_index];

        // remove the selected request from the queue
        for(int i = closest_index; i < *queue_cnt - 1; i++)
        {
            request_queue[i] = request_queue[i + 1];
        }

        (*queue_cnt)--;

        return next_request;
    }
}

struct RCB handle_request_arrival_look(struct RCB request_queue[QUEUEMAX],int *queue_cnt, struct RCB current_request, struct RCB new_request, int timestamp)
{
    if(current_request.request_id == 0)  // If the disk is free (current_request is NULLRCB)
    {
        return new_request;  // Service the newly-arriving request immediately
    }
    else
    {
        request_queue[*queue_cnt] = new_request;  // Add the newly-arriving request to the request queue
        (*queue_cnt)++;  // Increment the count of requests in the queue
        return current_request;  // Continue servicing the currently-serviced request
    }
}

struct RCB handle_request_completion_look(struct RCB request_queue[QUEUEMAX], int *queue_cnt, int current_cylinder, int scan_direction) {
    if(*queue_cnt == 0) {
        struct RCB nullRCB;
        nullRCB.request_id = -1;  // assign some kind of flag value to indicate this is a null RCB
        return nullRCB;
    }

    struct RCB nextRCB;
    int found = -1;
    if(scan_direction == 1) { // moving up
        for(int i = 0; i < *queue_cnt; i++) {
            if(request_queue[i].cylinder >= current_cylinder) {
                nextRCB = request_queue[i];
                found = i;
                break;
            }
        }
    } else if(scan_direction == 0) { // moving down
        for(int i = *queue_cnt - 1; i >= 0; i--) {
            if(request_queue[i].cylinder <= current_cylinder) {
                nextRCB = request_queue[i];
                found = i;
                break;
            }
        }
    }

    if(found != -1) { // If found, shift elements left to fill the gap
        for(int i = found; i < *queue_cnt - 1; i++) {
            request_queue[i] = request_queue[i + 1];
        }
        (*queue_cnt)--; // decrease queue count as one request is handled
    } else { // If not found, consider the direction
        if(scan_direction == 1) {
            nextRCB = request_queue[0];
            for(int i = 0; i < *queue_cnt - 1; i++) {
                request_queue[i] = request_queue[i + 1];
            }
            (*queue_cnt)--;
        } else if(scan_direction == 0) {
            nextRCB = request_queue[*queue_cnt - 1];
            (*queue_cnt)--;
        }
    }

    return nextRCB;
}


// int main() {
//     // Initialize the request queue and the counter
//     struct RCB request_queue[QUEUEMAX];
//     int queue_cnt = 0;

//     // Define a test case
//     struct RCB test_request = {1, 52, 58, 58, 1};

//     // Add the test request to the queue
//     request_queue[queue_cnt++] = test_request;

//     // Run the function on the queue
//     struct RCB result = handle_request_completion_look(request_queue, &queue_cnt, 52, 1);

//     // Check the correctness of the function
//     assert(result.request_id == test_request.request_id);
//     assert(result.arrival_timestamp == test_request.arrival_timestamp);
//     assert(result.cylinder == test_request.cylinder);
//     assert(result.address == test_request.address);
//     assert(result.process_id == test_request.process_id);

//     printf("All tests passed successfully.\n");

//     return 0;
// }

