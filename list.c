
#include "list.h"

#include <stdlib.h>
#include <string.h>

typedef struct _PrivateListData_t
{
    void* pListData;
    struct _PrivateListData_t* pNext;
} _PrivateListData;

_PrivateListData* GetEndNode(LinkedList* pList)
{
    _PrivateListData* pCurNode = (_PrivateListData*)pList->pListData;
    _PrivateListData* pPrevNode = NULL;
    while(pCurNode != NULL)
    {
        pPrevNode = pCurNode;
        pCurNode = pCurNode->pNext;
    }
    return pPrevNode;
}

LinkedList* CreateList()
{
    LinkedList* pReturnList = (LinkedList*)malloc(sizeof(LinkedList));
    memset(pReturnList, 0, sizeof(LinkedList));
    return pReturnList;
}

int32_t ListPush(LinkedList* pList, void* pData)
{
    if(pData == NULL)
    {
        return 0;
    }
    _PrivateListData* pNewNode = (_PrivateListData*)malloc(sizeof(_PrivateListData));
    ((_PrivateListData*)pNewNode)->pListData = pData;

    // push it to the back
    _PrivateListData* pEndNode = GetEndNode(pList);
    if(pEndNode == NULL)
    {
        // the list is empty
        ((_PrivateListData*)pList)->pListData = (void*)pNewNode;
    }
    else
    {
        pEndNode->pNext = pEndNode;
    }


    return 1;
}

void* ListPop(LinkedList* pList)
{
    // invalid or empty list
    if(pList == NULL || (_PrivateListData*)(pList->pListData) == NULL)
    {
        return NULL;
    }
    // save the data to return
    void* pDataReturn = ((_PrivateListData*)(pList->pListData))->pListData;

    // free and reset
    free(pList->pListData);
    pList->pListData = ((_PrivateListData*)(pList->pListData))->pNext;
    return pDataReturn;
}

void DestroyList(LinkedList* pList)
{
    _PrivateListData* pCurNode = (_PrivateListData*)ListPop(pList);
    while(pCurNode != NULL)
    {
        pCurNode = (_PrivateListData*)ListPop(pList);
    }

    free(pList);
}

#undef NULL
