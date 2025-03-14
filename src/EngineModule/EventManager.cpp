#include "pch.h"

#include "EventManager.h"

BOOL EventManager::Initialize(UINT maxBucketNum, UINT maxEventNum)
{
    m_pEventPool = new MemoryPool;
    m_pEventPool->Initialize(sizeof(Event), maxEventNum);

    m_pListenerPool = new MemoryPool;
    m_pListenerPool->Initialize(sizeof(ListenerInfo), maxEventNum);

    m_pHashTable = new HashTable;
    m_pHashTable->Initialize(maxBucketNum, MAX_NAME * sizeof(WCHAR), maxEventNum);

    return TRUE;
}

BOOL EventManager::PublishEvent(const WCHAR *evtID, const void *param, const float delayed)
{
    Event *pEvent = nullptr;

    const UINT keySize = wcslen(evtID) * sizeof(WCHAR);
    if (m_pHashTable->Select((void **)&pEvent, 1, evtID, keySize))
    {
        __debugbreak();
        return FALSE;
    }

    pEvent = (Event *)m_pEventPool->Alloc();
    if (!pEvent)
    {
        __debugbreak();
        return FALSE;
    }
    ZeroMemory(pEvent, sizeof(pEvent));

    pEvent->pEventID = evtID;
    pEvent->pArg = param;
    pEvent->delayTime = delayed;

    pEvent->pSearchHandle = m_pHashTable->Insert((void *)pEvent, evtID, keySize);
    if (!pEvent->pSearchHandle)
    {
        __debugbreak();
        return FALSE;
    }

    return TRUE;
}

BOOL EventManager::Subcribe(const WCHAR *evtID, const ListenerInfo &listenerInfo)
{
    // 유효성 및 중복 검사
    assert(listenerInfo.pListener);

    Event *pEvent = nullptr;

    const UINT keySize = wcslen(evtID) * sizeof(WCHAR);
    if (m_pHashTable->Select((void **)&pEvent, 1, evtID, keySize))
    {
        ListenerInfo *pInfo = (ListenerInfo *)m_pListenerPool->Alloc();
        *pInfo = listenerInfo;

        LinkToLinkedListFIFO(&pEvent->pListenerLinkHead, &pEvent->pListenerLinkTail, &pInfo->link);

        return TRUE;
    }
    return FALSE;
}

void EventManager::UnSubcribe(const WCHAR *evtID, const EventHandler *listener)
{
    // 유효성 및 중복 검사
    Event *pEvent = nullptr;

    const UINT keySize = wcslen(evtID) * sizeof(WCHAR);
    if (m_pHashTable->Select((void **)&pEvent, 1, evtID, keySize))
    {
        SORT_LINK *pCur = pEvent->pListenerLinkHead;
        while (pCur)
        {
            ListenerInfo *pInfo = (ListenerInfo *)pCur->pItem;

            if (pInfo->pListener == listener)
            {
                UnLinkFromLinkedList(&pEvent->pListenerLinkHead, &pEvent->pListenerLinkTail, &pInfo->link);
                m_pListenerPool->Dealloc(pInfo);
                return;
            }

            pCur = pCur->pNext;
        }
    }

#ifdef _DEBUG
    __debugbreak();
#endif // DEBUG

    return;
}

void EventManager::DeleteListener(const EventHandler *pListener) 
{ 
    BOOL insufficient;
    UINT itemNum = m_pHashTable->GetItemNum();

    Event **eventList = new Event *[itemNum];
    itemNum = m_pHashTable->GetAllItems((void **)eventList, itemNum, &insufficient);

    for (int i = 0; i < itemNum; i++)
    {
        Event     *pEvent = eventList[i];
        SORT_LINK *pCur = pEvent->pListenerLinkHead;
        while (pCur)
        {
            ListenerInfo *pInfo = (ListenerInfo *)pCur->pItem;

            if (pInfo->pListener == pListener)
            {
                UnLinkFromLinkedList(&pEvent->pListenerLinkHead, &pEvent->pListenerLinkTail, &pInfo->link);
                m_pListenerPool->Dealloc(pInfo);
                return;
            }

            pCur = pCur->pNext;
        }
    }

    #ifdef _DEBUG
    __debugbreak();
#endif // DEBUG

    return;
}
