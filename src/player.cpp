// The player, from 0x449ca0, as the Player half of th07's player.cpp.
// The shots follow in their own object.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

Player::Player()
{
}

int Player::CheckAreaHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    PlayerBombHitbox *area;
    int idx;
    // The point, from the area's centre, and turned into its frame.
    D3DXVECTOR3 d2;
    D3DXVECTOR3 rot;
    // A box's half size, or its corners.
    D3DXVECTOR3 half;
    Vector2 rel;
    D3DXVECTOR3 max;

    area = this->hitAreas;
    for (idx = 0; idx < 0xc0; idx++, area++)
    {
        if (!area->active)
        {
            continue;
        }
        if (area->radius != 0.0)
        {
            rel.x = pos->x - area->x;
            rel.y = pos->y - area->y;
            if (rel.x * rel.x + rel.y * rel.y < area->radius * area->radius)
            {
                goto hit;
            }
        }
        else if (area->angle != 0.0f)
        {
            d2.x = pos->x - area->x;
            d2.y = pos->y - area->y;
            Rotate2D(&rot, &d2, -area->angle);
            half.x = area->width / 2.0f;
            half.y = area->height / 2.0f;
            if (-half.x <= rot.x && rot.x <= half.x && -half.y <= rot.y && rot.y <= half.y)
            {
                goto hit;
            }
        }
        else
        {
            half.x = area->x - area->width / 2.0f;
            half.y = area->y - area->height / 2.0f;
            max.x = area->width / 2.0f + area->x;
            max.y = area->height / 2.0f + area->y;
            if (!(half.x > pos->x || max.x < pos->x || half.y > pos->y || max.y < pos->y))
            {
                goto hit;
            }
        }
    }
    return 0;

hit:
    this->areaItemType = area->itemType;
    area->hits++;
    return 2;
}

int Player::CheckHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    this->areaItemType = 6;
    if (this->CheckAreaHit(pos, size))
    {
        return 2;
    }
    start.x = pos->x - size->x / 2.0f;
    start.y = pos->y - size->y / 2.0f;
    end.x = size->x / 2.0f + pos->x;
    end.y = size->y / 2.0f + pos->y;
    if (this->hitboxMin.x > end.x || this->hitboxMin.y > end.y || this->hitboxMax.x < start.x ||
        this->hitboxMax.y < start.y)
    {
        return 0;
    }
    g_ReplayManager->frameEvents |= 2;
    if (this->playerState != 0)
    {
        return 1;
    }
    g_GameManager.RerollLifeChecksumKeys();
    this->Die();
    return 1;
}

int Player::CheckHitIgnoringAreas(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    this->areaItemType = 6;
    start.x = pos->x - size->x / 2.0f;
    start.y = pos->y - size->y / 2.0f;
    end.x = size->x / 2.0f + pos->x;
    end.y = size->y / 2.0f + pos->y;
    if (this->hitboxMin.x > end.x || this->hitboxMin.y > end.y || this->hitboxMax.x < start.x ||
        this->hitboxMax.y < start.y)
    {
        return 0;
    }
    g_ReplayManager->frameEvents |= 2;
    if (this->playerState != 0)
    {
        return 1;
    }
    g_GameManager.RerollLifeChecksumKeys();
    this->Die();
    return 1;
}

int Player::CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    this->areaItemType = 6;
    if (this->CheckAreaHit(pos, size))
    {
        return 2;
    }
    start.x = pos->x - size->x / 2.0f - 20.0f;
    start.y = pos->y - size->y / 2.0f - 20.0f;
    end.x = size->x / 2.0f + pos->x + 20.0f;
    end.y = size->y / 2.0f + pos->y + 20.0f;
    if (this->playerState == 2 || this->playerState == 1)
    {
        return 0;
    }
    if (this->grazeMin.x > end.x || this->grazeMax.x < start.x || this->grazeMin.y > end.y ||
        this->grazeMax.y < start.y)
    {
        return 0;
    }
    this->Graze(pos, 0);
    return 1;
}

int Player::TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    if (this->playerState != 0 && this->playerState != 3 && this->playerState != 4)
    {
        return 0;
    }
    start = *pos - *size / 2.0f;
    end = *pos + *size / 2.0f;
    if (this->grazeTestMin.x > end.x || this->grazeTestMax.x < start.x || this->grazeTestMin.y > end.y ||
        this->grazeTestMax.y < start.y)
    {
        return 0;
    }
    return 1;
}

int Player::CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 boxMin;
    D3DXVECTOR3 boxMax;

    // The hitbox, turned into the laser's frame.
    start = this->pos - *origin;
    Rotate2D(&end, &start, -angle);
    end.z = 0.0f;
    start = end + *origin;
    boxMin = start - this->hitboxSize;
    boxMax = start + this->hitboxSize;
    start = *pos - *size / 2.0f;
    end = *pos + *size / 2.0f;
    if (!(boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y))
    {
        goto hit;
    }
    if (!canGraze)
    {
        return 0;
    }
    start.x -= 48.0f;
    start.y -= 48.0f;
    end.x += 48.0f;
    end.y += 48.0f;
    if (boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y)
    {
        return 0;
    }
    if (this->playerState == 2 || this->playerState == 1)
    {
        return 0;
    }
    this->Graze(&this->pos, 1);
    return 2;

hit:
    g_ReplayManager->frameEvents |= 2;
    if (this->playerState != 0)
    {
        return 0;
    }
    g_GameManager.RerollLifeChecksumKeys();
    this->Die();
    return 1;
}

void Player::Graze(D3DXVECTOR3 *pos, int isLaser)
{
    D3DXVECTOR3 mid;
    int count;
    int points;

    if (!g_Unknown17d6ed4)
    {
        count = g_GameManager.IsUnk22AtLow1() ? 3 : (g_GameManager.IsUnk22AtLow2() ? 2 : 1);
        if (g_GameManager.stats->graze < 99999)
        {
            g_GameManager.stats->graze += count;
        }
        if (g_GameManager.stats->grazeTotal < 999999)
        {
            g_GameManager.stats->grazeTotal += count;
        }
    }
    mid = (this->pos + *pos) / 2.0f;
    g_EffectManager.SpawnEffect(8, &mid, 1, 0xffffffff);
    g_GameManager.AddSubRank(6);
    g_Gui.grazeChanged = 2;
    g_SoundPlayer.QueueSoundPanned(0x1e, pos->x);
    points = g_GameManager.IsUnk22AtHigh2() ? 4000 : 2000;
    g_GameManager.AddScore(points);
    if (this->IsUnk5Set())
    {
        g_GameManager.Func43c0bb(100, 0);
    }
    if ((!g_GameManager.IsSoloHuman() || g_GameManager.character == 10) && g_EnemyManager.AnyBossActive() &&
        g_GameManager.IsUnk22AtHigh1())
    {
        g_ItemManager.SpawnItem(pos, 10, 1);
        if (!isLaser && g_SpellCard.Func4178a0())
        {
            g_ItemManager.SpawnItem(pos, 10, 1);
            if (!g_GameManager.IsSoloYoukai())
            {
                g_ItemManager.SpawnItem(pos, 10, 1);
            }
        }
    }
    g_ReplayManager->frameEvents |= 0x2000;
}

void Player::Die()
{
    Effect *effect;

    DebugPrint("player DEAD");
    g_GameManager.isFrozen = 0;
    g_GameManager.RerollChecksumKeys();
    g_EffectManager.SpawnEffect(6, &this->pos, 16, 0xffffffff);
    this->playerState = 2;
    this->unk_e2af4 = 0;
    g_SoundPlayer.QueueSoundPanned(4, this->pos.x);
    g_ReplayManager->frameEvents |= 0x200;
    if (g_GameManager.unk_3dbac_7 != 0)
    {
        DebugPrint(" desolve\n");
        this->deathbombFrames = 2;
        this->unk_6 = 1;
    }
    else
    {
        g_GameManager.SetUnk22(0);
        if (g_GameManager.GetBombs() >= 1)
        {
            this->deathbombFrames = g_GameManager.GetBombs() * 6;
            if (g_GameManager.GetUnk3c() >= g_GameManager.GetUnk40())
            {
                this->deathbombFrames += 7;
            }
            if (this->deathbombFrames > 15)
            {
                this->deathbombFrames = 15;
            }
            if (g_SpellCard.Func4178a0())
            {
                this->deathbombFrames += this->deathbombFrames;
                if (this->deathbombFrames > 30)
                {
                    this->deathbombFrames = 30;
                }
            }
            if (g_GameManager.character == 0 || g_GameManager.character == 4 || g_GameManager.character == 5)
            {
                this->deathbombFrames *= 9;
                this->deathbombFrames /= 5;
            }
            DebugPrint(" preDeadCount %d\n", this->deathbombFrames);
            this->sprite.color2Rgb[2] = 0xff;
            this->sprite.color2Rgb[1] = 0xff;
            this->sprite.color2Rgb[0] = 0xff;
            this->sprite.color2Alpha = this->sprite.alpha;
            this->sprite.useColor2 = 1;
            this->unk_e2b28 = g_EffectManager.SpawnReservedEffect(0x3b, &this->pos, 0xb, 1, 0xfff0404f);
            effect = this->unk_e2b28;
            effect->sprite.interpDuration[0] = 0;
            effect->sprite.interpFrame[0] = this->deathbombFrames;
            effect->sprite.interpEasing = 4;
            effect->sprite.moveStart.x = 128.0f;
            effect->sprite.moveEnd.x = 8.0f;
            effect->sprite.moveStart.y = 32.0f;
            effect->sprite.moveEnd.y = 0.0f;
            effect->sprite.pos.x = 128.0f;
            effect->sprite.pos.y = 32.0f;
            effect->segmentCount = 0x40;
            effect->angle = 0.0f;
            effect->radius = 128.0f;
            effect->ringWidth = 15.0f;
            effect->unk_334 = 6.0f;
            effect->unk_357 = 1;
            if (g_SpellCard.Func4178a0())
            {
                g_GameManager.unk_3dbac_10 = 1;
            }
        }
        else
        {
            this->deathbombFrames = 2;
            DebugPrint(" Miss\n");
        }
    }
    g_ItemManager.RecycleCollectedItems();
}

float Player::AngleFrom(D3DXVECTOR3 *pos)
{
    float x;
    float y;

    x = this->pos[0] - pos->x;
    y = this->pos[1] - pos->y;
    if (y == 0.0f && x == 0.0f)
    {
        return 1.5707964f;
    }
    return atan2f(y, x);
}

int Player::RegisterChain(int arg)
{
    Player *player = &g_Player;
    ShotTypeData *oldShotType;
    ShotTypeData *oldShotType2;

    if (KeepsResources())
    {
        oldShotType = player->shotType;
        oldShotType2 = player->unk_e2a78;
    }
    memset(player, 0, sizeof(Player));
    if (KeepsResources())
    {
        player->shotType = oldShotType;
        player->unk_e2a78 = oldShotType2;
    }
    player->unk_e2af4 = 0;
    player->registerChainArg = arg;
    player->calcChain = g_Chain.CreateElem((ChainCallback)Player::OnUpdate);
    player->calcChain->arg = player;
    player->calcChain->addedCallback = (ChainCallback)Player::AddedCallback;
    player->calcChain->deletedCallback = (ChainCallback)Player::DeletedCallback;
    if (g_Chain.AddToCalcChain(player->calcChain, 9) != 0)
    {
        return -1;
    }
    player->drawChain = g_Chain.CreateElem((ChainCallback)Player::OnDraw);
    player->drawChain2 = g_Chain.CreateElem((ChainCallback)Player::OnDraw2);
    player->drawChain->arg = player;
    player->drawChain2->arg = player;
    g_Chain.AddToDrawChain(player->drawChain, 9);
    g_Chain.AddToDrawChain(player->drawChain2, 10);
    return 0;
}

int Player::OnUpdate(Player *player)
{
    if (g_GameManager.isFrozen)
    {
        if (player->unk_be834 != NULL)
        {
            player->unk_be834->sprite.scriptPaused = 1;
        }
        if (player->unk_e2b24 != NULL)
        {
            player->unk_e2b24->sprite.scriptPaused = 1;
        }
        return 1;
    }
    if (player->unk_be834 != NULL)
    {
        player->unk_be834->sprite.scriptPaused = 0;
    }
    if (player->unk_e2b24 != NULL)
    {
        player->unk_e2b24->sprite.scriptPaused = 0;
    }
    UpdateHitboxAreas(player);
    UpdateBombState(player);
    if (player->playerState == 2)
    {
        if (UpdateDyingState(player))
        {
            goto respawn;
        }
    }
    else if (player->playerState == 1)
    {
    respawn:
        UpdateRespawningState(player);
    }
    UpdateInvulnerability(player);
    if (player->playerState != 2 && player->playerState != 1)
    {
        HandleInput(player);
    }
    g_SpriteEngine->Step(&player->sprite);
    UpdateBullets(player);
    UpdateFiring(player);
    ResetHomingAndText(player);
    if (!g_Gui.IsMsgRunning())
    {
        g_GameManager.unk_3de10++;
        g_GameManager.unk_3de14++;
        if (g_GameManager.IsUnk22AtLow1())
        {
            g_GameManager.unk_3de1c++;
            g_GameManager.unk_3de24++;
            g_GameManager.AddScore(100);
        }
        else if (g_GameManager.IsUnk22AtHigh1())
        {
            g_GameManager.unk_3de18++;
            g_GameManager.unk_3de20++;
            g_GameManager.AddScore(100);
        }
    }
    return 1;
}

void UpdateHitboxAreas(Player *player)
{
    PlayerBombHitbox *area;
    int i;

    // Both arrays, bombHitboxes and hitAreas.
    area = player->bombHitboxes;
    for (i = 0; i < 0x180; i++, area++)
    {
        if (area->framesLeft < 0)
        {
            continue;
        }
        area->framesLeft--;
        area->radius += area->radiusGrowth;
        area->width += area->widthGrowth;
        area->height += area->heightGrowth;
        if (area->framesLeft <= 0)
        {
            area->Deactivate();
        }
    }
}

void UpdateBombState(Player *player)
{
    int forced;
    DWORD i;

    forced = 0;
    if (player->unk_6 && player->deathbombFrames == 1)
    {
        // The last words run out: a bomb starts on its own.
        forced = 1;
        goto start;
    }
    if (player->bombCooldown != 0)
    {
        player->bombCooldown--;
    }
    if (player->bomb.running)
    {
        if (player->bomb.timer.HasTicked())
        {
            g_Gui.frameItemsHideTimer = 2;
        }
        if (player->bomb.timer >= player->bomb.duration)
        {
            g_SpellCard.Func416130();
            player->bomb.running = 0;
            player->moveScaleX = player->moveScaleY = 1.0f;
            if (player->bomb.type == 4)
            {
                g_GameManager.unk_3dbac_7 = 0;
                for (i = 0; i < 8; i++)
                {
                    if (g_EnemyManager.bosses[i] != NULL)
                    {
                        g_EnemyManager.bosses[i]->Func42adb0(0);
                        g_EnemyManager.bosses[i]->unk_2dfc = 0;
                        g_EnemyManager.bosses[i]->unk_3324_30 = 0;
                    }
                }
                ScreenEffect::StartEffect(3, 0x1e, 1, -1, 0, 0x15);
            }
        }
        else
        {
            player->bomb.update.funcs[player->bomb.type](player);
            player->bomb.timer++;
        }
        if (player->bomb.type < 4)
        {
            if (player->bomb.type & 1)
            {
                g_GameManager.Func43c0bb(26000 / player->bomb.duration, 1);
            }
            else
            {
                g_GameManager.Func43c0bb(-26000 / player->bomb.duration, 1);
            }
        }
        return;
    }
    if ((g_GameButtons & 2) && !g_GameManager.ChecksumMismatch() && !g_Gui.IsMsgRunning() &&
        player->deathbombFrames != 0 && g_GameManager.GetBombs() > 0 && player->bombCooldown == 0)
    {
        if (g_GameManager.unk_3dbac_7 != 0 || g_GameManager.isInSpellCardPractice)
        {
            if ((g_GameButtons & 2) && (g_GameButtons & 2) != (g_PrevGameButtons & 2))
            {
                g_SoundPlayer.QueueSound(0x29, 0);
            }
            return;
        }
    start:
        g_ReplayManager->frameEvents |= 1;
        player->unk_6 = 0;
        if (g_GameManager.unk_3dbac_7 != 0)
        {
            player->bomb.type = 4;
        }
        else
        {
            player->sprite.useColor2 = 0;
            if (player->unk_e2b28 != NULL)
            {
                player->unk_e2b28->active = 0;
                player->unk_e2b28 = NULL;
            }
            g_GameManager.unk_3dbac_10 = 0;
            g_SpriteEngine->ResetColorModulation();
            player->bomb.type = player->unk_3;
            if (player->unk_4)
            {
                player->bomb.type = 1 - player->bomb.type;
            }
            if (player->unk_4)
            {
                // Last words: up to two bombs.
                player->bomb.type += 2;
                if (forced)
                {
                    player->bomb.bombsUsed = g_GameManager.GetBombs();
                    g_GameManager.SetBombs(0);
                }
                else if (g_GameManager.GetBombs() < 2)
                {
                    player->bomb.bombsUsed = g_GameManager.GetBombs();
                    g_GameManager.SetBombs(0);
                }
                else
                {
                    player->bomb.bombsUsed = 2;
                    g_GameManager.AddBombs(-2);
                }
                g_GameManager.hscr.lastSpells++;
            }
            else
            {
                g_GameManager.hscr.bombs++;
                g_GameManager.AddBombs(-1);
            }
            g_GameManager.AddBombsUsed(1);
        }
        player->unk_4 = 0;
        g_Gui.bombsChanged = 2;
        player->bomb.running = 1;
        player->bombJustStarted = 1;
        player->bomb.timer = 0;
        player->bomb.duration = 999;
        player->bomb.update.funcs[player->bomb.type](player);
        player->bomb.timer++;
        g_GameManager.SubSubRank(200);
        g_SpellCard.Func44cba0();
        player->deathbombFrames += 6;
        if (player->deathbombFrames > g_Player.shotType->deathbombFrames)
        {
            player->deathbombFrames = g_Player.shotType->deathbombFrames;
        }
    }
    else
    {
        player->bombJustStarted = 0;
    }
}

int UpdateDyingState(Player *player)
{
    float progress;

    if (player->deathbombFrames != 0)
    {
        // The last words: they cost time orbs while they run.
        g_GameManager.AddTimeOrbs(-15);
        player->deathbombFrames--;
        player->unk_4 = 1;
        if (player->deathbombFrames == 0)
        {
            if (player->unk_e2b28 != NULL)
            {
                player->unk_e2b28->active = 0;
                player->unk_e2b28 = NULL;
            }
            g_EffectManager.SpawnReservedEffect(0xc, &player->pos, 3, 1, 0xff4040ff);
            g_EffectManager.SpawnEffect(6, &player->pos, 16, 0xffffffff);
            g_SoundPlayer.QueueSoundPanned(0xf, player->pos.x);
            g_GameManager.unk_3dbac_10 = 0;
            g_SpriteEngine->ResetColorModulation();
            player->sprite.useColor2 = 0;
            g_ReplayManager->frameEvents |= 4;
            g_GameManager.unk_3dba8 = 0;
            player->unk_4 = 0;
            g_SpellCard.Func44d150();
            g_GameManager.AddDeaths(1);
            g_Gui.unk_4_10 = 2;
            g_GameManager.AddTimeOrbs(g_GameManager.stats->unk_3c > 5000 ? -500 : -g_GameManager.stats->unk_3c / 10);
            if (g_GameManager.GetLives() > 0)
            {
                if (g_GameManager.GetPower() <= 16)
                {
                    g_GameManager.SetPower(0);
                }
                else
                {
                    g_GameManager.AddPower(-16);
                }
                g_ItemManager.SpawnItem(&player->pos, 2, 2);
                g_ItemManager.SpawnItem(&player->pos, 0, 2);
                g_ItemManager.SpawnItem(&player->pos, 0, 2);
                g_ItemManager.SpawnItem(&player->pos, 0, 2);
                g_ItemManager.SpawnItem(&player->pos, 0, 2);
                g_ItemManager.SpawnItem(&player->pos, 0, 2);
                if (g_GameManager.GetBombs() > 0 &&
                    (g_GameManager.character == 2 || g_GameManager.character == 8 || g_GameManager.character == 9))
                {
                    g_ItemManager.SpawnItem(&player->pos, 3, 2);
                }
                g_Gui.powerChanged = 2;
                g_ItemManager.RecycleCollectedItems();
            }
            else
            {
                g_GameManager.SetPower(0);
                g_ItemManager.SpawnItem(&player->pos, 4, 2);
                g_ItemManager.SpawnItem(&player->pos, 4, 2);
                g_ItemManager.SpawnItem(&player->pos, 4, 2);
                g_ItemManager.SpawnItem(&player->pos, 4, 2);
                g_ItemManager.SpawnItem(&player->pos, 4, 2);
                g_Gui.powerChanged = 2;
            }
            g_GameManager.SubSubRank(1600);
        }
    }
    else
    {
        progress = player->unk_e2af4.AsFloat() / 30.0f;
        player->sprite.scaleX = 3.0f * progress + 1.0f;
        player->sprite.scaleY = 1.0f - 1.0f * progress;
        player->sprite.color = (int)(255.0f - player->unk_e2af4.AsFloat() * 255.0f / 30.0f) << 24 | 0xffffff;
        player->sprite.SetBlendModeAdd();
        player->velocityX = 0.0f;
        player->velocityY = 0.0f;
        if (player->unk_e2af4.AsFrames() >= 30)
        {
            player->playerState = 1;
            player->pos[0] = g_GameManager.arena[2] / 2.0f;
            player->pos[1] = g_GameManager.arena[3] - 64.0f;
            player->pos[2] = 0.2f;
            player->unk_e2af4 = 0;
            player->sprite.scaleY = 3.0f;
            player->sprite.scaleX = 3.0f;
            if ((g_GameManager.character < 4 && !player->unk_3) || !(g_GameManager.character & 1))
            {
                player->anm->StartScript(&player->sprite, 0);
            }
            else
            {
                player->anm->StartScript(&player->sprite, 5);
            }
            if (g_GameManager.GetLives() <= 0)
            {
                g_GameManager.isInContinueMenu = 1;
            }
            else
            {
                g_GameManager.AddLives(-1);
                g_Gui.livesChanged = 2;
                g_GameManager.SetBombs(g_Player.shotType->bombs);
                g_Gui.bombsChanged = 2;
                return 1;
            }
        }
    }
    return 0;
}

void UpdateRespawningState(Player *player)
{
    float progress;

    player->respawnClearFrames = 60;
    progress = 1.0f - player->unk_e2af4.AsFloat() / 30.0f;
    player->sprite.scaleX = 2.0f * progress + 1.0f;
    player->sprite.scaleY = 1.0f - 1.0f * progress;
    player->sprite.SetBlendModeAdd();
    player->moveScaleY = 1.0f;
    player->moveScaleX = 1.0f;
    player->sprite.color = player->unk_e2af4.AsFrames() * 255 / 30 << 24 | 0xffffff;
    player->deathbombFrames = 0;
    if (player->unk_e2af4.AsFrames() >= 30)
    {
        player->playerState = 3;
        player->sprite.scaleY = 1.0f;
        player->sprite.scaleX = 1.0f;
        player->sprite.color = 0xffffffff;
        player->sprite.SetBlendModeNormal();
        if (!g_GameManager.isInSpellCardPractice)
        {
            player->unk_e2af4 = 240;
        }
        player->deathbombFrames = g_Player.shotType->deathbombFrames;
    }
}

void UpdateInvulnerability(Player *player)
{
    if (player->respawnClearFrames != 0)
    {
        player->respawnClearFrames--;
        // A hit area over the whole play area clears the bullets.
        player->Func44de60(&player->pos, 768.0f, 896.0f, -1, 0);
    }
    if (player->playerState == 3)
    {
        player->unk_4 = 0;
        if (player->invulnerabilityEffect != NULL)
        {
            player->invulnerabilityEffect->pos = player->pos;
        }
        player->unk_e2af4--;
        if (player->unk_e2af4.AsFrames() <= 0)
        {
            if (player->invulnerabilityEffect != NULL)
            {
                player->invulnerabilityEffect->active = 0;
                player->invulnerabilityEffect = NULL;
            }
            player->playerState = 0;
            player->unk_e2af4 = 0;
            player->sprite.color = 0xffffffff;
        }
        else if (player->unk_e2af4.AsFrames() % 8 < 2)
        {
            player->sprite.color = 0xfff02020;
        }
        else
        {
            player->sprite.color = 0xffffffff;
        }
    }
    else
    {
        player->unk_e2af4++;
    }
}

void ResetHomingAndText(Player *player)
{
    player->orbHomingTarget = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->homingTarget = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->homingTargetSet = 0;
    if (player->pos.y >= 400.0f)
    {
        if (g_AsciiText.GetSummaryPendingInterrupt() != 2 && player->pos.x < 160.0f)
        {
            g_AsciiText.SetSummaryPendingInterrupt(2);
        }
        else if (g_AsciiText.GetSummaryPendingInterrupt() == 2 && player->pos.x > 160.0f)
        {
            g_AsciiText.SetSummaryPendingInterrupt(3);
        }
    }
    else if (g_AsciiText.GetSummaryPendingInterrupt() == 2)
    {
        g_AsciiText.SetSummaryPendingInterrupt(3);
    }
}

int Player::OnDraw(Player *player)
{
    DWORD i;

    DrawBullets(player);
    if (player->bomb.running)
    {
        player->bomb.draw.funcs[player->bomb.type](player);
    }
    if (!g_GameManager.isInContinueMenu)
    {
        player->sprite.pos.x = g_GameManager.arena[0] + player->pos.x;
        player->sprite.pos.y = g_GameManager.arena[1] + player->pos.y;
        player->sprite.pos.z = 0.1f;
        g_SpriteEngine->DrawUnrotated(&player->sprite);
    }
    for (i = 0; i < 4; i++)
    {
        if (player->options[i].draw != NULL)
        {
            player->options[i].draw(player, &player->options[i]);
        }
    }
    return 1;
}

int Player::OnDraw2(Player *player)
{
    DrawHitBullets(player);
    return 1;
}
