#pragma once

#include <any>

class EventHandler;
struct Event
{
    const WCHAR *pEventID;
    const void  *pArg;

    SORT_LINK *pListenerLinkHead;
    SORT_LINK *pListenerLinkTail;
    void      *pSearchHandle;

    float delayTime;

    Event(const WCHAR *pID, const void *arg, const float delay) : pEventID(pID), pArg(arg), delayTime(delay)
    {
        pListenerLinkHead = nullptr;
        pListenerLinkTail = nullptr;
        pSearchHandle = nullptr;
    }
};

struct ListenerInfo
{
    const EventHandler         *pListener = nullptr;
    std::function<void(void *)> func;

    SORT_LINK link;
    ListenerInfo(const EventHandler *pInListener, const std::function<void(void *)> &inFunc)
        : pListener(pInListener), func(inFunc)
    {
        link.pPrev = nullptr;
        link.pNext = nullptr;
        link.pItem = this;
    }
};

class EventManager
{
    MemoryPool *m_pEventPool = nullptr;
    MemoryPool *m_pListenerPool = nullptr;
    HashTable *m_pHashTable = nullptr;

  public:
    BOOL Initialize(UINT maxBucketNum, UINT maxEventNum);

    BOOL PublishEvent(const WCHAR *evtID, const void *param, const float delayed);
    BOOL Subcribe(const WCHAR *evtID, const ListenerInfo &listenerInfo);
    void UnSubcribe(const WCHAR *evtID, const EventHandler *listener);
    void DeleteListener(const EventHandler *pListener);
};
