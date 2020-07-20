
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

typedef void* Queue;

// gives you an empty FILO list
Queue queue_create();

// destroy the Queue
// NOTE: you need to handle anything the Queue is holding.  i suggest doing this yourself
void queue_destroy(Queue queue);

// return true(1) if data was added, else false(0)
bool queue_push_front(Queue queue, void* data);
bool queue_push_back(Queue queue, void* data);

// returns the top data and removes it from the queue
void* queue_pop_front(Queue queue); // O(1)
void* queue_pop_back(Queue queue); // O(N)

// returns the top data
void* queue_peek_front(const Queue queue);
void* queue_peek_back(const Queue queue);

// return true(1) if empty, else false(0)
// will return true if invalid
bool queue_is_empty(Queue queue);

uint32_t queue_size(Queue queue);

#endif // _QUEUE_H_
