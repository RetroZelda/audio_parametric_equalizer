
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdint.h>
#include <stdbool.h>

typedef void* Array;

// gives you an empty array
// in:
//      capacity_hint - gives a hint for the start capacity
//      enforce_capacity - if true, wont add elements beyond the capacity
Array array_create(uint32_t capacity_hint, bool enforce_capacity);

// destroy the array
// NOTE: you need to handle anything the array is holding.
void array_destroy(Array array);

// clear the array but keep the capacity
bool array_clear(Array array);

// make the capacity fit the current size
bool array_make_fit(Array array);

// return true(1) if data was added, else false(0)
bool array_push_front(Array array, void* data);
bool array_push_back(Array array, void* data);
bool array_insert(Array array, uint32_t index, void* data);

// returns the top data and removes it from the array
void* array_pop_front(Array array);
void* array_pop_back(Array array);
void* array_remove(Array array, uint32_t index);

// returns the top data
void* array_get(const Array array, uint32_t index);

uint32_t array_size(Array array);
uint32_t array_capacity(Array array);


#endif // _ARRAY_H_
