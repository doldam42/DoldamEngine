#include "pch.h"
#include "LinkedList.h"

void LinkToLinkedList(SORT_LINK **ppHead, SORT_LINK **ppTail, SORT_LINK *pNew)
{
    if (!*ppHead)
    {
        *ppTail = *ppHead = pNew;
        (*ppHead)->pNext = nullptr;
        (*ppHead)->pPrev = nullptr;
    }
    else
    {
        // #ifdef _DEBUG
        if (*ppHead == pNew)
            __debugbreak();
        // #endif
        pNew->pNext = (*ppHead);
        (*ppHead)->pPrev = pNew;
        (*ppHead) = pNew;
        pNew->pPrev = nullptr;
    }
}

void LinkToLinkedListFIFO(SORT_LINK **ppHead, SORT_LINK **ppTail, SORT_LINK *pNew)
{
    if (!*ppHead)
    {
        *ppTail = *ppHead = pNew;
        (*ppHead)->pNext = nullptr;
        (*ppHead)->pPrev = nullptr;
    }
    else
    {
        pNew->pPrev = (*ppTail);
        (*ppTail)->pNext = pNew;
        (*ppTail) = pNew;
        pNew->pNext = nullptr;
    }
}

void UnLinkFromLinkedList(SORT_LINK **ppHead, SORT_LINK **ppTail, SORT_LINK *pDel)
{
    SORT_LINK *pPrv = pDel->pPrev;
    SORT_LINK *pNext = pDel->pNext;

#ifdef _DEBUG
    if (pDel->pPrev)
    {
        if (pDel->pPrev->pNext != pDel)
            __debugbreak();
    }
#endif

    if (pDel->pPrev)
        pDel->pPrev->pNext = pDel->pNext;
    else
    {
#ifdef _DEBUG
        if (pDel != (*ppHead))
            __debugbreak();
#endif
        (*ppHead) = pNext;
    }

    if (pDel->pNext)
        pDel->pNext->pPrev = pDel->pPrev;
    else
    {
#ifdef _DEBUG
        if (pDel != (*ppTail))
            __debugbreak();
#endif
        (*ppTail) = pPrv;
    }
    pDel->pPrev = nullptr;
    pDel->pNext = nullptr;
}
