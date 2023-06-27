#include "oslabs.h"
#include <limits.h>
#include <stdlib.h>
#include<stdio.h>

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

struct RCB handle_request_completion_look(struct RCB request_queue[QUEUEMAX], int *queue_cnt, int current_cylinder, int scan_direction)
{
    if (*queue_cnt == 0)  // If the request queue is empty
    {
        struct RCB NULLRCB = {0, 0, 0, 0, 0};  
        return NULLRCB;  // Return NULLRCB to indicate that there is no request to service next
    }
    else
    {
        int target_index;
        int minimum_difference = INT_MAX;  // Initialize minimum_difference with the maximum possible integer
        int earliest_arrival_time = INT_MAX;  // Initialize earliest_arrival_time with the maximum possible integer

        for (int i = 0; i < *queue_cnt; i++)
        {
            int cylinder_difference = abs(current_cylinder - request_queue[i].cylinder);

            // If the request is in the same direction as the scan direction, or if there is no request in the scan direction
            if ((scan_direction == 1 && request_queue[i].cylinder >= current_cylinder) || 
                (scan_direction == 0 && request_queue[i].cylinder <= current_cylinder) ||
                (scan_direction == 1 && minimum_difference > current_cylinder) ||
                (scan_direction == 0 && minimum_difference > QUEUEMAX - current_cylinder))
            {
                if (cylinder_difference < minimum_difference || 
                    (cylinder_difference == minimum_difference && request_queue[i].arrival_timestamp < earliest_arrival_time))
                {
                    // If the current request has a closer cylinder or has the same cylinder but an earlier arrival time
                    target_index = i;  // Pick this request
                    minimum_difference = cylinder_difference;
                    earliest_arrival_time = request_queue[i].arrival_timestamp;
                }
            }
        }

        struct RCB next_request = request_queue[target_index];  // Store the selected request

        // Remove the selected request from the request queue
        for (int i = target_index; i < *queue_cnt - 1; i++)
        {
            request_queue[i] = request_queue[i + 1];
        }
        (*queue_cnt)--;  // Decrease the count of requests in the queue

        return next_request;  // Return the selected request
    }
}

