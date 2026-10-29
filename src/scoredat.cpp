// The score file ("score.dat"): loading it and reading its chunks. th07
// has these at the start of its /Os resultscreen.cpp; th08 compiles them
// as an /Od object of their own, right after the result screen's.

#include <string.h>

#include "resultscreen.h"
#include "unsorted.h"

int InsertScore(ListNode *head, Hscr *entry)
{
    int index = 0;
    ListNode *next;

    while (head->next != NULL)
    {
        if (head->next->data != NULL && head->next->data->score <= entry->score)
        {
            break;
        }
        head = head->next;
        index++;
    }
    next = head->next;
    head->next = (ListNode *)g_MemoryManager.Alloc(sizeof(ListNode), "result");
    head->next->prev = head;
    head = head->next;
    head->data = entry;
    head->next = next;
    return index;
}

void FreeListNodes(ListNode *head)
{
    ListNode *next;

    head = head->next;
    while (head != NULL)
    {
        next = head->next;
        g_MemoryManager.Free(head);
        head = next;
    }
}
