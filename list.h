
#ifndef _LIST_H_
#define _LIST_H_

#include <stdint.h>

typedef struct
{
    void* pListData;
} LinkedList;

// gives you an empty FIFO list
LinkedList* CreateList();

// return true(1) if data was added, else false(0)
// O(n)
int32_t ListPush(LinkedList* pList, void* pData);

// returns the top data
// O(1)
void* ListPop(LinkedList* pList);

// destroy the list
// NOTE: you need to handle anything the list is holding.  i suggest doing this yourself
// O(n)
void DestroyList(LinkedList* pList);

#endif // _LIST_H_
