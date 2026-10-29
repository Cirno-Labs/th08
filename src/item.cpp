// The items, 0x440010 to 0x4418bb, as th07's item.cpp.

#include "bullet.h"
#include "item.h"
#include "player.h"
#include "unsorted.h"

ItemManager::ItemManager()
{
}

Item *ItemManager::SpawnItem(D3DXVECTOR3 *pos, int type, int state)
{
    int i;
    Item *item;

    item = &this->items[this->nextIndex];
    if (pos->x < -64.0f || pos->x > 448.0f)
    {
        return &this->items[0x830];
    }
    if (g_GameManager.GetPower() >= 128 && (type == 0 || type == 2))
    {
        type = 8;
    }
    if (type == 7)
    {
        state = 3;
    }
    else if (type == 10)
    {
        state = 5;
        type = 7;
    }
    for (i = 0; i < 0x830; i++)
    {
        this->nextIndex++;
        if (item->active)
        {
            if (this->nextIndex >= 0x830)
            {
                this->nextIndex = 0;
                item = this->items;
            }
            else
            {
                item++;
            }
            if (type == 7)
            {
                return &this->items[0x830];
            }
            continue;
        }
        if (this->nextIndex >= 0x830)
        {
            this->nextIndex = 0;
        }
        item->active = 1;
        item->pos = *pos;
        item->velocity.x = 0.0f;
        item->velocity.y = -2.2f;
        item->velocity.z = 0.0f;
        item->type = type;
        item->state = state;
        item->timer = 0;
        if (state == 2)
        {
            item->target.x = g_Rng.GetRandomF32InRange(288.0f) + 48.0f;
            item->target.y = g_Rng.GetRandomF32InRange(192.0f) - 64.0f;
            item->target.z = 0.0f;
            item->velocity = item->pos;
        }
        else if (state == 3)
        {
            item->velocity.y = -2.0f - g_Rng.GetRandomF32InRange(0.2f);
            item->velocity.x = g_Rng.GetRandomF32SignedInRange(0.6f);
            if (g_Player.playerState == 2)
            {
                item->state = 0;
                item->velocity.x = 0.0f;
                item->velocity.y = -0.9f;
                item->velocity.z = 0.0f;
            }
        }
        else if (state == 5)
        {
            item->velocity.y = -2.0f - g_Rng.GetRandomF32InRange(0.2f);
            item->velocity.x = g_Rng.GetRandomF32SignedInRange(0.6f);
            if (g_Player.playerState == 2)
            {
                item->state = 0;
                item->velocity.x = 0.0f;
                item->velocity.y = -0.9f;
                item->velocity.z = 0.0f;
            }
        }
        g_BulletManager.etamaAnm->StartScript(&item->sprite, type + 0x3d);
        item->sprite.color = 0xffffffff;
        item->sprite.flags |= 0x2000;
        item->countsFullValue = 0;
        item->onScreen = 1;
        this->activeListTail->next = item;
        item->prev = this->activeListTail;
        item->next = NULL;
        this->activeListTail = item;
        break;
    }
    return i < 0x830 ? item : &this->items[0x830];
}

void Func440470()
{
    if (g_GameManager.difficulty < 4)
    {
        if ((DWORD)g_GameManager.stats->unk_34 < 6)
        {
            g_GameManager.stats->unk_38 = g_Unknown4c7a84[g_GameManager.stats->unk_34];
        }
        else
        {
            g_GameManager.stats->unk_38 = (g_GameManager.stats->unk_34 - 5) * 500 + g_Unknown4c7a84[5];
        }
    }
    else if ((DWORD)g_GameManager.stats->unk_34 < 3)
    {
        g_GameManager.stats->unk_38 = g_Unknown4c7a9c[g_GameManager.stats->unk_34];
    }
    else
    {
        g_GameManager.stats->unk_38 = 99999;
    }
}

void ItemManager::CollectAll()
{
    Item *entry = this->activeList.next;

    while (entry != NULL)
    {
        entry->state = 1;
        entry->velocity = D3DXVECTOR3(0.0f, -0.5f, 0.0f);
        entry = entry->next;
    }
}
