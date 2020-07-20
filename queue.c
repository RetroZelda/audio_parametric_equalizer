
#include "queue.h"

#include <stdlib.h>
#include <string.h>

typedef struct _queue_node
{
    void* data;
    struct _queue_node* next;
} queue_node;

typedef struct _queue_descriptor
{
    struct _queue_node* head;
    struct _queue_node* tail;
    uint32_t size;
} queue_descriptor;

Queue queue_create()
{
    Queue new_queue = (Queue)malloc(sizeof(queue_descriptor));
    memset(new_queue, 0, sizeof(queue_descriptor));
    return new_queue;
}

void queue_destroy(Queue queue)
{
    while(!queue_is_empty(queue))
    {
        queue_pop_front(queue);
    }

    free(queue);
}

bool queue_push_front(Queue queue, void* data)
{
    if(queue == NULL)
        return false;

    queue_descriptor* queue_info = (queue_descriptor*)queue;
    queue_node* new_node = (queue_node*)malloc(sizeof(queue_node));
    if(new_node != NULL)
    {
        new_node->data = data;
        new_node->next = queue_info->head;
        queue_info->head = new_node;
        queue_info->size++;

        // handle adding the first node
        if(queue_info->tail == NULL)
            queue_info->tail = new_node;

        return true;
    }
    return false;
}

bool queue_push_back(Queue queue, void* data)
{
    if(queue == NULL)
        return false;

    queue_descriptor* queue_info = (queue_descriptor*)queue;
    queue_node* new_node = (queue_node*)malloc(sizeof(queue_node));
    if(new_node != NULL)
    {
        new_node->data = data;
        new_node->next = NULL;

        // handle first node
        if(queue_info->head == NULL)
        {
            queue_info->head = new_node;
        }
        else
        {
            queue_info->tail->next = new_node;
        }
        queue_info->tail = new_node;
        queue_info->size++;

        return true;
    }
    return false;
}

void* queue_pop_front(Queue queue)
{
    // invalid or empty Queue
    if(queue_is_empty(queue))
        return NULL;

    queue_descriptor* queue_info = (queue_descriptor*)queue;
    queue_node* head_node = queue_info->head;
    void* return_data = head_node->data;
    queue_info->head = head_node->next;
    queue_info->size--;

    // if we were the last node, clear
    if(queue_info->tail == head_node)
        queue_info->tail = NULL;

    // free and reset
    free(head_node);
    return return_data;
}

void* queue_pop_back(Queue queue)
{
    // invalid or empty Queue
    if(queue_is_empty(queue))
        return NULL;

    queue_descriptor* queue_info = (queue_descriptor*)queue;
    queue_node* tail_node = queue_info->tail;

    // handle if we are the last node
    if(queue_info->head == tail_node)
    {
        memset(queue_info, 0, sizeof(queue_descriptor));
    }
    else
    {
        // find the next tail
        queue_node* new_tail = queue_info->head;
        while(new_tail->next != tail_node)
        {
            new_tail = new_tail->next;
        }

        // set the next tail
        queue_info->tail = new_tail;
        new_tail->next = NULL;
        queue_info->size--;
    }

    // free and reset
    void* return_data = tail_node->data;
    free(tail_node);
    return return_data;
}

void* queue_peek_front(const Queue queue)
{
    if(queue_is_empty(queue))
        return NULL;

    queue_descriptor* queue_info = (queue_descriptor*)queue;
    return queue_info->head->data;
}

void* queue_peek_back(const Queue queue)
{
    if(queue_is_empty(queue))
        return NULL;

    queue_descriptor* queue_info = (queue_descriptor*)queue;
    return queue_info->tail->data;
}


bool queue_is_empty(Queue queue)
{
    return queue_size(queue) == 0;
}

uint32_t queue_size(Queue queue)
{
    if(queue == NULL)
        return 0;
    queue_descriptor* queue_info = (queue_descriptor*)queue;
    return queue_info->size;
}