
#include "array.h"

#include <stdlib.h>
#include <string.h>

#define ARRAY_HEADER_UINT32 0x41524159 // "ARAY"

typedef struct _array_node
{
    void* data;
} array_node;

typedef struct _array_descriptor
{
    uint32_t check;
    uint32_t size;
    uint32_t capacity;
    bool enforce_capacity;
    array_node data[];
} array_descriptor;

typedef struct _array_data
{
    array_descriptor* info;
} array_data;

bool is_valid_array(const array_data* array_info)
{
    return array_info 
        && array_info->info 
        && array_info->info->check == ARRAY_HEADER_UINT32;
}

bool is_valid_index(const array_data* array_info, uint32_t index)
{
    return array_info 
        && array_info->info 
        && array_info->info->size > index;
}

bool has_space(const array_data* array_info)
{
    return array_info 
        && array_info->info 
        && (array_info->info->size < array_info->info->capacity || !array_info->info->enforce_capacity);
}

bool grow_array(array_data* array_info)
{
    if(!is_valid_array(array_info) || array_info->info->enforce_capacity)
        return false;

    if(array_info->info->capacity == 0)
        array_info->info->capacity = 1;

    array_descriptor* new_space = (array_descriptor*)realloc(array_info->info, sizeof(array_descriptor) + (sizeof(array_node) * array_info->info->capacity * 2));
    if(!new_space)
        return false;

    array_info->info = new_space;
    array_info->info->capacity *= 2;
    return true;
}

Array array_create(uint32_t capacity_hint, bool enforce_capacity)
{
    // allocate the array
    array_data* new_array = (array_data*)malloc(sizeof(array_data));
    if(!new_array)
        return NULL;

    if(capacity_hint == 0) // dont let stupid be stupid
        capacity_hint = 1;

    // allocate the new descriptor
    array_descriptor* array_info = (array_descriptor*)malloc(sizeof(array_descriptor) + (sizeof(array_node) * capacity_hint));
    if(!array_info)
    {
        free(new_array);
        return NULL;
    }

    // build the data
    memset(array_info, 0, sizeof(array_descriptor) + capacity_hint);
    array_info->check = ARRAY_HEADER_UINT32;
    array_info->capacity = capacity_hint;
    array_info->enforce_capacity = enforce_capacity;

    // set and return
    new_array->info = array_info;
    return(Array)new_array;
}

void array_destroy(Array array)
{
    array_data* array_info = (array_data*)array;
    if(is_valid_array(array_info))
    {
        array_info->info->check = 0;
        free(array_info->info);
        free(array_info);
    }
}

bool array_clear(Array array)
{
    array_data* array_info = (array_data*)array;
    if(is_valid_array(array_info))
    {
        array_info->info->size = 0;
        return true;
    }
    return false;
}

bool array_make_fit(Array array)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info) || array_info->info->enforce_capacity || array_info->info->size == 0)
        return false;

    array_descriptor* new_space = (array_descriptor*)realloc(array_info->info, sizeof(array_descriptor) + (sizeof(array_node) * array_info->info->size));
    if(!new_space)
        return false;

    array_info->info = new_space;
    array_info->info->capacity = array_info->info->size;
    return true;
}

bool array_push_front(Array array, void* data)
{
    return array_insert(array, 0, data);
}

bool array_push_back(Array array, void* data)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info) || !has_space(array_info))
        return false;
    
    // handle growing the array
    if(array_info->info->size + 1 >= array_info->info->capacity)
    {
        if(!grow_array(array_info))
            return false;
    }
    array_info->info->data[array_info->info->size++].data = data;
    return true;
}

bool array_insert(Array array, uint32_t index, void* data)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info) || !has_space(array_info))
        return false;

    // handle growing the array
    if(array_info->info->size + 1 > array_info->info->capacity)
    {
        if(!grow_array(array_info))
            return false;
    }

    if(array_info->info->size > 0)
    {
        // shift everything past and including 'index'
        array_node* shift_dest = &array_info->info->data[array_info->info->size];
        for(uint32_t pos_count = 0; pos_count < array_info->info->size - index; ++pos_count)
        {
            uint32_t source_index = array_info->info->size - pos_count - 1;
            array_node* shift_source = &array_info->info->data[source_index];
            shift_dest->data = shift_source->data;
            shift_dest = shift_source;
        }
    }
    array_info->info->data[index].data = data;
    array_info->info->size++;
    return true;
}

void* array_pop_front(Array array)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info) || array_info->info->size == 0)
        return NULL;
    
    void* return_data = array_info->info->data[0].data;
    if(array_info->info->size-- > 1)
    {
        // shift everything
        array_node* shift_dest = &array_info->info->data[0];
        for(int32_t source_index = 1; source_index <= array_info->info->size; ++source_index)
        {
            array_node* shift_source = &array_info->info->data[source_index];
            shift_dest->data = shift_source->data;
            shift_dest = shift_source;
        }
    }

    return return_data;
}

void* array_pop_back(Array array)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info) || array_info->info->size == 0)
        return NULL;
    return array_info->info->data[--array_info->info->size].data;
}

void* array_remove(Array array, uint32_t index)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info) || !is_valid_index(array_info, index) || array_info->info->size == 0)
        return NULL;

    void* return_data = array_info->info->data[index].data;
    if(array_info->info->size-- > 1 && index != array_info->info->size ) // not the last item or the last index
    {
        // shift everything
        array_node* shift_dest = &array_info->info->data[index];
        for(int32_t source_index = index + 1; source_index <= array_info->info->size; ++source_index)
        {
            array_node* shift_source = &array_info->info->data[source_index];
            shift_dest->data = shift_source->data;
            shift_dest = shift_source;
        }
    }

    return return_data;
    
}

void* array_get(const Array array, uint32_t index)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info) || !is_valid_index(array_info, index))
        return NULL;
    return array_info->info->data[index].data;
}

uint32_t array_size(Array array)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info))
        return 0;
    return array_info->info->size;
}

uint32_t array_capacity(Array array)
{
    array_data* array_info = (array_data*)array;
    if(!is_valid_array(array_info))
        return 0;
    return array_info->info->capacity;
}
