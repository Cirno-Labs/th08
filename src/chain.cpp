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

void Chain::ReleaseList(ChainElem *root)
{
    ChainElem list;
    ChainElem *tail;
    ChainElem *tmp;
    ChainElem *cur;

    tail = (ChainElem *)g_MemoryManager.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
    list.next = tail;
    cur = root;
    while (cur != NULL)
    {
        tail->self = cur;
        tail->next = (ChainElem *)g_MemoryManager.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
        tail = tail->next;
        cur = cur->next;
    }
    cur = &list;
    while (cur != NULL)
    {
        this->Remove(cur->self);
        cur = cur->next;
    }
    tail = list.next;
    while (tail != NULL)
    {
        tmp = tail->next;
        g_MemoryManager.Unregister(tail);
        delete tail;
        tail = NULL;
        tail = tmp;
    }
}

void Chain::Release()
{
    g_App.CloseSubThread();
    this->ReleaseList(&this->calcChain);
    this->ReleaseList(&this->drawChain);
}

ChainElem *Chain::CreateElem(ChainCallback callback)
{
    ChainElem *elem = (ChainElem *)g_MemoryManager.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
    elem->SetCallback(callback);
    elem->allocated = 1;
    return elem;
}

void Chain::Remove(ChainElem *elem)
{
    g_App.EnterCriticalSection(0);
    this->RemoveLocked(elem);
    g_App.LeaveCriticalSection(0);
}

void Chain::RemoveLocked(ChainElem *elem)
{
    BOOL isDraw = FALSE;
    ChainElem *cur;
    ChainCallback callback;

    if (elem == NULL)
    {
        return;
    }
    cur = &this->calcChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    isDraw = TRUE;
    cur = &this->drawChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    return;

found:
    // isDraw is set but no longer used: th06 logged which chain the
    // element was cut from.
    if (elem->prev != NULL)
    {
        elem->callback = NULL;
        elem->prev->next = elem->next;
        if (elem->next != NULL)
        {
            elem->next->prev = elem->prev;
        }
        elem->prev = NULL;
        elem->next = NULL;
        if (elem->allocated)
        {
            // The element's deleted callback runs outside the section.
            g_App.LeaveCriticalSection(0);
            g_MemoryManager.Unregister(elem);
            delete elem;
            elem = NULL;
            g_App.EnterCriticalSection(0);
        }
        else if (elem->deletedCallback != NULL)
        {
            callback = elem->deletedCallback;
            elem->deletedCallback = NULL;
            g_App.LeaveCriticalSection(0);
            callback(elem->arg);
            g_App.EnterCriticalSection(0);
        }
    }
}
