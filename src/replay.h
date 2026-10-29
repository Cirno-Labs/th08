// Replays. Only what is known so far.

#pragma once

// Saves the replay being recorded to `path` under `name`; with NULLs it
// frees the replay data instead, as th07's (0x4531f0).
void SaveReplay(char *path, char *name);
