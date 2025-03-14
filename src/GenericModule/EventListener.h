#pragma once

#include "LinkedList.h"

class EventHandler;

struct GameEvent
{
    std::function<void(void*)> func;
    void *arg = nullptr;

    SORT_LINK link;
};

class EventListener
{
    SORT_LINK *m_pEventLinkHead = nullptr;
    SORT_LINK *m_pEventLinkTail = nullptr;

  public:
    void Run();

    GameEvent *AddCallback(const std::function<void(void*)> func , void *arg, size_t sizeOfArg);

    void DeleteCallback(GameEvent *pEvent);
    void DeleteAllCallback();

    EventListener() = default;
    ~EventListener();
};
