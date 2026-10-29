// The chains that run the game: every frame, one runs the update
// callbacks of its elements and the other their draw callbacks, in
// order of priority.

#pragma once

#include <windows.h>

#include "common.h"

// Chain callbacks take the element's `arg`. They return a status; 1
// keeps the element and moves on to the next.
// With /Gr they are __fastcall, taking `arg` in ecx.
typedef int (*ChainCallback)(void *arg);

// An element of a chain. Allocated with new by Chain::CreateElem.
struct ChainElem
{
    ChainElem();
    // Calls the deleted callback, if any, and clears the links and
    // callbacks.
    ~ChainElem();

    // Lower runs first.
    short priority;
    // Set by Chain::CreateElem: the element was allocated with new.
    unsigned short allocated : 1;
    ChainCallback callback;
    // Called when the element is added to a chain, which clears it
    // after the call.
    ChainCallback addedCallback;
    ChainCallback deletedCallback;
    ChainElem *prev;
    ChainElem *next;
    // Points to the element itself; in ReleaseList's copy of a list, to
    // the element copied.
    ChainElem *self;
    void *arg;
};
ASSERT_SIZE(ChainElem, 0x20);

// The global at 0x164f548: the heads of the update and draw chains, as
// th07's.
class Chain
{
public:
    Chain();
    ~Chain();

    // Calls `elem`'s added callback, clearing it, then inserts `elem`
    // into the update chain by priority, inside critical section 0.
    // Returns the callback's result, or 0 without one.
    int AddToCalcChain(ChainElem *elem, int priority);
    // Like AddToCalcChain, for the draw chain.
    int AddToDrawChain(ChainElem *elem, int priority);
    // Runs the update chain's callbacks in order, removing the elements
    // whose callback returns 0. A callback returns 2 to run again, 3, 4 or
    // 5 to stop with 1, 0 or -1, and 6 to start over. Returns how many
    // ran. Holds critical section 0 except while a callback runs.
    int RunCalcChain();
    // Like RunCalcChain for the draw chain, without starting over.
    int RunDrawChain();
    // Like Remove, for a caller that holds critical section 0. Leaves it
    // while the element is deleted or its deleted callback runs. New in
    // th08.
    void RemoveLocked(ChainElem *elem);

    ChainElem calcChain;
    ChainElem drawChain;
};
ASSERT_SIZE(Chain, 0x40);
extern Chain g_Chain;
