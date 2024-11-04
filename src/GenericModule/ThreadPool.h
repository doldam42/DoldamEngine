#pragma once

#include <Windows.h>
#include <stdio.h>

struct ThreadPoolTask
{
    void  (*function)(void *arg);
    void *arg;
};

struct ThreadDesc;
struct ThreadPool
{
    HANDLE *hThreads;
    HANDLE  hCompleteEvent;

    ThreadDesc *threadDescList;

    ThreadPoolTask *taskQueue;
    size_t          taskQueueSize;
    size_t          taskCount;

    CRITICAL_SECTION lock;
    size_t           activeThreadCount;
    size_t           maxThreadCount;

    BOOL shutdown;
};

struct ThreadDesc
{
    ThreadPool *pool;
    HANDLE      hProcessEvent;
    UINT        threadIndex;
};

DWORD WINAPI ThreadFunction(LPVOID param)
{
    ThreadDesc *desc = (ThreadDesc *)param;

    ThreadPool *pool = desc->pool;
    UINT        threadIndex = desc->threadIndex;

    while (TRUE)
    {
        WaitForSingleObject(desc->hProcessEvent, INFINITE);
        // Queue 안의 모든 task가 소진될 때까지 반복
        while (TRUE)
        {
            EnterCriticalSection(&pool->lock);
            if (pool->taskCount == 0 && pool->shutdown)
            {
                LeaveCriticalSection(&pool->lock);
                return 0;
            }
            if (pool->taskCount == 0)
            {
                LeaveCriticalSection(&pool->lock);
                Sleep(0);
                break;
            }

            ThreadPoolTask task = pool->taskQueue[--pool->taskCount];

            LeaveCriticalSection(&pool->lock);

            task.function(task.arg);

            if (InterlockedDecrement(&pool->activeThreadCount) == 0 &&
                InterlockedCompareExchange(&pool->taskCount, 0, 0) == 0)
            {
                SetEvent(pool->hCompleteEvent);
            }
        }
    }
    return 0;
}

void ThreadPoolCleanup(ThreadPool *pool)
{
    if (pool)
    {
        if (pool->hThreads)
        {
            for (size_t i = 0; i < pool->maxThreadCount; i++)
            {
                if (pool->hThreads[i])
                {
                    CloseHandle(pool->hThreads[i]);
                    pool->hThreads[i] = nullptr;
                }
            }
            delete[] pool->hThreads;
            pool->hThreads = nullptr;
        }
        if (pool->threadDescList)
        {
            for (size_t i = 0; i < pool->maxThreadCount; i++)
            {
                if (pool->threadDescList->hProcessEvent)
                {
                    CloseHandle(pool->threadDescList[i].hProcessEvent);
                    pool->threadDescList[i].hProcessEvent = nullptr;
                }
            }
            delete[] pool->threadDescList;
            pool->threadDescList = nullptr;
        }

        if (pool->hCompleteEvent)
        {
            CloseHandle(pool->hCompleteEvent);
            pool->hCompleteEvent = nullptr;
        }
        if (pool->taskQueue)
        {
            delete[] pool->taskQueue;
            pool->taskQueue = nullptr;
        }
        delete pool;
    }
}

ThreadPool *ThreadPoolCreate(size_t maxThreads, size_t taskQueueSize)
{
    ThreadPool *pool = new ThreadPool;
    if (pool == NULL)
    {
        return nullptr;
    }

    pool->taskQueueSize = taskQueueSize;
    pool->taskCount = 0;
    pool->activeThreadCount = 0;
    pool->maxThreadCount = maxThreads;

    pool->hThreads = new HANDLE[maxThreads];
    if (pool->hThreads == NULL)
    {
        ThreadPoolCleanup(pool);
        return nullptr;
    }

    pool->threadDescList = new ThreadDesc[maxThreads];
    if (pool->threadDescList == NULL)
    {
        ThreadPoolCleanup(pool);
        return nullptr;
    }
    memset(pool->threadDescList, 0, sizeof(ThreadDesc) * maxThreads);

    pool->taskQueue = new ThreadPoolTask[taskQueueSize];
    if (pool->taskQueue == NULL)
    {
        ThreadPoolCleanup(pool);
        return nullptr;
    }

    pool->hCompleteEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (pool->hCompleteEvent == NULL)
    {
        ThreadPoolCleanup(pool);
        return nullptr;
    }

    InitializeCriticalSection(&pool->lock);

    for (size_t i = 0; i < maxThreads; i++)
    {
        pool->threadDescList[i].pool = pool;
        pool->threadDescList[i].threadIndex = i;
        pool->threadDescList[i].hProcessEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        pool->hThreads[i] = CreateThread(NULL, 0, ThreadFunction, &pool->threadDescList[i], 0, NULL);

        if (pool->hThreads[i] == NULL || pool->threadDescList[i].hProcessEvent == NULL)
        {
            DeleteCriticalSection(&pool->lock);
            ThreadPoolCleanup(pool);
            return nullptr;
        }
    }

    return pool;
}

void ThreadPoolAddTask(ThreadPool *pool, void (*function)(void *), void *arg)
{
    EnterCriticalSection(&pool->lock);

    if (pool->taskCount < pool->taskQueueSize)
    {
        ThreadPoolTask task;
        task.function = function;
        task.arg = arg;
        pool->taskQueue[pool->taskCount++] = task;
        InterlockedIncrement(&pool->activeThreadCount);
    }

    LeaveCriticalSection(&pool->lock);
}

void ThreadPoolProcess(ThreadPool *pool)
{
    for (size_t i = 0; i < pool->maxThreadCount; i++)
    {
        SetEvent(pool->threadDescList[i].hProcessEvent);
    }
}

void ThreadPoolWait(ThreadPool *pool) { WaitForSingleObject(pool->hCompleteEvent, INFINITE); }

void ThreadPoolDestroy(ThreadPool *pool)
{
    EnterCriticalSection(&pool->lock);
    pool->shutdown = TRUE;
    LeaveCriticalSection(&pool->lock);

    for (size_t i = 0; i < pool->maxThreadCount; i++)
    {
        WaitForSingleObject(pool->hThreads[i], INFINITE);
    }

    DeleteCriticalSection(&pool->lock);
    ThreadPoolCleanup(pool);
}
