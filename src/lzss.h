// The LZSS coder of the score file and the data archives: a 0x2000-byte
// window, 13-bit offsets and 4-bit lengths (3 to 18 bytes), matches found
// through a binary search tree over the window. Not in th06, whose
// archives decode inside DatArchive::Decode.

#pragma once

#include <windows.h>

#include "common.h"

// A node of the search tree: one per window position, and a root at
// 0x2000. Links are window positions, 0 for none.
struct LzssNode
{
    int parent;
    int smallerChild;
    int largerChild;
};
ASSERT_SIZE(LzssNode, 0xc);

// The tree; its root is g_LzssTree[0x2000].
extern LzssNode g_LzssTree[0x2001];
// The last 0x2000 bytes coded.
extern BYTE g_LzssWindow[0x2000];

// Compresses `size` bytes at `data` into a new GlobalAlloc block, which
// it returns, and stores the compressed size in `outSize`.
void *LzssCompress(BYTE *data, int size, int *outSize);
// Decompresses `dataSize` bytes at `data` into `out`, which holds
// `outSize` bytes, or into a new GlobalAlloc block of that size if `out`
// is NULL. Returns the output, or NULL if it cannot be allocated.
void *LzssDecompress(BYTE *data, int dataSize, BYTE *out, int outSize);
// Clears the window and the tree.
void LzssClearWindow();
// Makes window position `pos` the tree's only node.
void LzssTreeInitNode(int pos);
// Adds the string at window position `pos` to the tree. Returns the
// length of the longest match found and stores its position in
// `matchPosition`.
int LzssInsertNode(int pos, int *matchPosition);
// Removes the string at window position `pos` from the tree.
void LzssDeleteNode(int pos);
// Removes node `oldNode`, which has at most one child, `newNode`, from
// the tree, linking the child to its parent.
void LzssReplaceNode(int oldNode, int newNode);
// Puts node `newNode` in the place of `oldNode` in the tree.
void LzssReplaceChild(int oldNode, int newNode);
// The node that comes before `node`: the largest in its smaller subtree.
int LzssPredecessor(int node);
