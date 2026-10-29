// The chains, from 0x43c710.

#include "chain.h"
#include "unsorted.h"

Chain::~Chain()
{
}

ChainElem::ChainElem()
{
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->self = this;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
    this->priority = 0;
    this->allocated = 0;
}

ChainElem::~ChainElem()
{
    if (this->deletedCallback != NULL)
    {
        this->deletedCallback(this->arg);
    }
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
}

Chain::Chain()
{
}

int Chain::AddToCalcChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->calcChain;
    int result = 0;

    if (elem->addedCallback != NULL)
    {
        result = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;
    }
    g_App.EnterCriticalSection(0);
    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    g_App.LeaveCriticalSection(0);
    return result;
}

int Chain::AddToDrawChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->drawChain;
    int result = 0;

    if (elem->addedCallback != NULL)
    {
        result = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;
    }
    g_App.EnterCriticalSection(0);
    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    g_App.LeaveCriticalSection(0);
    return result;
}

int Chain::RunCalcChain()
{
    ChainElem *current;
    int updateCount;
    ChainElem *cut;
    int result;

    g_App.EnterCriticalSection(0);
restart:
    updateCount = 0;
    current = &this->calcChain;
    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        again:
            g_App.LeaveCriticalSection(0);
            result = current->callback(current->arg);
            g_App.EnterCriticalSection(0);
            switch (result)
            {
            case 0:
                cut = current;
                current = current->next;
                this->RemoveLocked(cut);
                updateCount++;
                continue;
            case 2:
                goto again;
            case 4:
                updateCount = 0;
                goto end;
            case 3:
                updateCount = 1;
                goto end;
            case 5:
                updateCount = -1;
                goto end;
            case 6:
                goto restart;
            default:
                updateCount++;
                break;
            }
        }
        current = current->next;
    }
end:
    g_App.LeaveCriticalSection(0);
    return updateCount;
}

int Chain::RunDrawChain()
{
    ChainElem *current;
    int updateCount;
    ChainElem *cut;
    int result;

    updateCount = 0;
    current = &this->drawChain;
    g_App.EnterCriticalSection(0);
    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        again:
            g_App.LeaveCriticalSection(0);
            result = current->callback(current->arg);
            g_App.EnterCriticalSection(0);
            switch (result)
            {
            case 0:
                cut = current;
                current = current->next;
                this->RemoveLocked(cut);
                updateCount++;
                continue;
            case 2:
                goto again;
            case 4:
                updateCount = 0;
                goto end;
            case 3:
                updateCount = 1;
                goto end;
            case 5:
                updateCount = -1;
                goto end;
            default:
                updateCount++;
                break;
            }
        }
        current = current->next;
    }
end:
    g_App.LeaveCriticalSection(0);
    return updateCount;
}
