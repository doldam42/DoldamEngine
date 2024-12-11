#include "pch.h"

#include "EventListener.h"

void EventListener::Run()
{ 
    SORT_LINK *pCur = m_pEventLinkHead;
    while (pCur)
    {
        GameEvent *pEvent = (GameEvent *)pCur->pItem;
        
        pEvent->func(pEvent->arg);

        pCur = pCur->pNext;
    }
}

// event의 경우 argumet에 대한 size를 가지고 있어야 될거 같음...
GameEvent *EventListener::AddCallback(const std::function<void(void *)> func, void *arg, size_t sizeOfArg)
{
    GameEvent *pEvent = new GameEvent;
    pEvent->func = func;
    
    if (arg)
    {
        pEvent->arg = new BYTE[sizeOfArg];
        memcpy(pEvent->arg, arg, sizeOfArg);
    }

    pEvent->link.pItem = pEvent;
    pEvent->link.pNext = nullptr;
    pEvent->link.pPrev = nullptr;
    LinkToLinkedListFIFO(&m_pEventLinkHead, &m_pEventLinkTail, &pEvent->link);

    return pEvent;
}

void EventListener::DeleteCallback(GameEvent *pEvent)
{
    UnLinkFromLinkedList(&m_pEventLinkHead, &m_pEventLinkTail, &pEvent->link);
    
    if (pEvent->arg)
    {
        delete pEvent->arg;
        pEvent->arg = nullptr;
    }

    delete pEvent;
}

void EventListener::DeleteAllCallback() 
{
    while (m_pEventLinkHead)
    {
        GameEvent *pEvent = (GameEvent*)(m_pEventLinkHead->pItem);
        DeleteCallback(pEvent);
    }
}

EventListener::~EventListener() { DeleteAllCallback(); }
