#pragma once

#include "LinkedList.h"

struct GameEvent
{
    void  (*func)(void *);
    void *arg = nullptr;

    SORT_LINK link;
};

class EventListener
{
    SORT_LINK *m_pEventLinkHead = nullptr;
    SORT_LINK *m_pEventLinkTail = nullptr;

  public:
    void Run();

    GameEvent *AddCallback(void (*func)(void *), void *arg, size_t sizeOfArg);

    void DeleteCallback(GameEvent *pEvent);
    void DeleteAllCallback();

    EventListener() = default;
    ~EventListener();
};