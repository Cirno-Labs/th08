// The items, 0x440010 to 0x4418bb, as th07's item.cpp.

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
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

void Item::CollectPower()
{
    int prevLevel;
    int level;

    if (g_GameManager.GetPower() >= 128)
    {
    }
    else
    {
        level = 0;
        while (g_GameManager.GetPower() >= g_PowerLevels[level])
        {
            level++;
        }
        prevLevel = level;
        g_GameManager.unk_3dba8 = 0;
        g_GameManager.AddPower(1);
        if (g_GameManager.GetPower() >= 128)
        {
            g_GameManager.SetPower(128);
            if (!g_SpellCard.Func4178a0())
            {
                g_BulletManager.Func415c60();
            }
            g_Gui.ShowFullPowerPopup(0, 1);
            g_ItemManager.ConvertPowerItems(this);
        }
        g_GameManager.AddScore(10);
        g_Gui.powerChanged = 2;
        while (g_GameManager.GetPower() >= g_PowerLevels[level])
        {
            level++;
        }
        if (level != prevLevel)
        {
            g_AsciiText.AddPopup(&this->pos, -1, 0xffffc0a0);
            g_SoundPlayer.QueueSound(0x1f, 0);
        }
        else
        {
            g_AsciiText.AddPopup(&this->pos, 10, 0xffffffff);
        }
    }
    g_GameManager.AddSubRank(1);
}

void Item::CollectPoint()
{
    int maxValue = g_GameManager.stats->unk_24;
    int amount;

    amount = (this->pos.y < g_Player.shotType->itemAutoCollectLine ? 1 : 0)
                 ? maxValue
                 : maxValue / 2 - (int)(this->pos.y - g_Player.shotType->itemAutoCollectLine) *
                                      (g_GameManager.stats->unk_24 / 1000);
    if (this->countsFullValue == 1)
    {
        amount = maxValue;
    }
    amount -= amount % 10;
    if (g_GameManager.IsUnk22AtLow1())
    {
        amount += amount;
    }
    g_AsciiText.AddPopup(&this->pos, amount, amount >= maxValue ? 0xffffff00 : 0xffffffff);
    if (amount >= maxValue)
    {
        this->countsFullValue = 1;
    }
    g_GameManager.AddScore(amount);
    g_GameManager.stats->unk_2c++;
    g_GameManager.stats->pointItemsTotal++;
    g_Gui.frameItemsHideTimer = 2;
    if (amount >= maxValue)
    {
        g_GameManager.AddSubRank(10);
    }
    else
    {
        g_GameManager.AddSubRank(3);
    }
    if (g_GameManager.stats->unk_34 >= 0)
    {
        for (;;)
        {
            Func440470();
            if (g_GameManager.stats->pointItemsTotal >= g_GameManager.stats->unk_38)
            {
                g_GameManager.AddLifeOrBomb();
                g_GameManager.stats->unk_34++;
                continue;
            }
            break;
        }
    }
    g_GameManager.hscr.pointItems++;
    g_GameManager.RerollChecksumKeys();
}

void Item::CollectFullPowerItem()
{
    int maxValue = g_GameManager.stats->unk_24;
    int amount;

    amount = (this->pos.y < g_Player.shotType->itemAutoCollectLine ? 1 : 0)
                 ? maxValue
                 : maxValue / 2 - (int)(this->pos.y - g_Player.shotType->itemAutoCollectLine) *
                                      (g_GameManager.stats->unk_24 / 1000);
    if (this->countsFullValue == 1)
    {
        amount = maxValue;
    }
    maxValue /= 10;
    maxValue -= maxValue % 10;
    amount /= 10;
    amount -= amount % 10;
    if (g_GameManager.IsUnk22AtLow1())
    {
        amount += amount;
    }
    g_AsciiText.AddPopup(&this->pos, amount, amount >= maxValue ? 0xffffff00 : 0xffffffff);
    g_GameManager.AddScore(amount);
    if (amount >= maxValue)
    {
        this->countsFullValue = 1;
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

void ItemManager::ConvertPowerItems(Item *except)
{
    Item *entry = this->activeList.next;

    while (entry != NULL)
    {
        if (entry != except && (entry->type == 0 || entry->type == 2))
        {
            if (entry->velocity.y > -0.5f)
            {
                entry->velocity.x = 0.0f;
                entry->velocity.y = -0.5f;
                entry->velocity.z = 0.0f;
            }
            g_EffectManager.SpawnEffect(0, &entry->pos, 1, 0xffffffff);
            entry->type = 8;
            g_BulletManager.etamaAnm->StartScript(&entry->sprite, 0x45);
        }
        entry = entry->next;
    }
}

void ItemManager::RecycleCollectedItems()
{
    Item *entry = this->activeList.next;

    while (entry != NULL)
    {
        if (entry->state == 1)
        {
            entry->state = 0;
            entry->velocity.x = 0.0f;
            entry->velocity.y = -0.9f;
            entry->velocity.z = 0.0f;
        }
        entry = entry->next;
    }
}

void ItemManager::DrawItems()
{
    Item *entry = this->activeList.next;
    int alpha;

    while (entry != NULL)
    {
        entry->sprite.pos.x = g_GameManager.arena[0] + entry->pos.x;
        entry->sprite.pos.y = g_GameManager.arena[1] + entry->pos.y;
        entry->sprite.pos.z = 0.15f;
        if (entry->pos[1] < -8.0f)
        {
            entry->sprite.pos.y = 8.0f + g_GameManager.arena[1];
            if (entry->onScreen)
            {
                g_BulletManager.etamaAnm->SetSprite(&entry->sprite, entry->type + 0xb6);
                entry->onScreen = 0;
                entry->sprite.flags |= 0x2000;
            }
            alpha = 255 - (int)((8.0f - entry->pos[1]) * 255.0f / 128.0f);
            if (alpha < 64)
            {
                alpha = 64;
            }
            entry->sprite.color = entry->sprite.color & 0xffffff | alpha << 24;
        }
        else if (!entry->onScreen)
        {
            g_BulletManager.etamaAnm->SetSprite(&entry->sprite, entry->type + 0xac);
            entry->onScreen = 1;
            entry->sprite.color = 0xffffffff;
            entry->sprite.flags |= 0x2000;
        }
        g_SpriteEngine->Draw(&entry->sprite);
        entry = entry->next;
    }
}

void Item::Unlink()
{
    this->active = 0;
    this->prev->next = this->next;
    if (this->next != NULL)
    {
        this->next->prev = this->prev;
    }
    if (g_ItemManager.activeListTail == this)
    {
        g_ItemManager.activeListTail = this->prev;
    }
}

int ItemManager::CountType7Items()
{
    Item *entry = this->activeList.next;
    int count = 0;

    while (entry != NULL)
    {
        if (entry->type == 7)
        {
            count++;
        }
        entry = entry->next;
    }
    return count;
}
