// The enemy script's function callbacks (g_EnemyScriptFuncs), from
// 0x423390 (SetScriptFunc0Vars, its first entry) to 0x425410, as th06's
// enemyscriptpattern.cpp splits its callbacks off enemyscript.cpp. The
// seam follows th06; th08's bytes neither show nor rule it out.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "unsorted.h"

void SetScriptFunc0Vars(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_AsciiText.ringAlpha = enemy->currentCtx->vars.ints[0];
    g_AsciiText.ringRadius = enemy->currentCtx->vars.floats[0];
}

void FlashScreen60(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 60, 1, 0xffffffff, 0, 0x15);
}

void BounceOffSides(Enemy *enemy, EnemyScriptInstr *instr)
{
    int bounced = 0;

    if (enemy->pos.x <= 0.0f || enemy->pos.x >= 384.0f)
    {
        enemy->velocity.x = -enemy->velocity.x;
        bounced = 1;
    }
    if (enemy->velocity.y < enemy->currentCtx->vars.floats[7])
    {
        enemy->velocity.y += enemy->currentCtx->vars.floats[6];
        bounced = 1;
    }
    if (enemy->pos.y < -64.0f)
    {
        enemy->velocity.y = -enemy->velocity.y;
        bounced = 1;
    }
    else if (enemy->pos.y >= 480.0f)
    {
        enemy->staysOffscreen = 0;
    }
    if (bounced)
    {
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
    }
}

void SpawnTwinEffects38(Enemy *enemy, EnemyScriptInstr *instr)
{
    Effect *effect;

    effect = g_EffectManager.SpawnReservedEffect(0x38, &enemy->pos, 9, 1, 0xffffffff);
    effect = g_EffectManager.SpawnReservedEffect(0x38, &enemy->pos, 10, 1, 0xffffffff);
    g_EffectManager.anm->StartScript(&effect->sprite, 0x61);
    g_StageManager.spellCardDrawCallback = DrawTwinEffects;
}

void DrawTwinEffects()
{
    Effect *first;
    D3DCOLOR saved;
    float r1;
    int i;
    Effect *second;
    Sprite *spr;
    float r2;
    VertexDiffuseXyzrwh vertices[10];

    first = g_EffectManager.GetReservedEffect(9);
    second = g_EffectManager.GetReservedEffect(10);
    spr = &g_StageManager.dialogueSprites[0];
    r1 = first->sprite.pos.x * 0.70710683f;
    r2 = second->sprite.pos.x * 0.70710683f;
    vertices[0].pos.x = 32.0f + first->pos.x - r1;
    vertices[0].pos.y = 16.0f + first->pos.y - r1;
    vertices[1].pos.x = 32.0f + first->pos.x - r2;
    vertices[1].pos.y = 16.0f + first->pos.y - r2;
    vertices[2].pos.x = 32.0f + first->pos.x + r1;
    vertices[2].pos.y = 16.0f + first->pos.y - r1;
    vertices[3].pos.x = 32.0f + first->pos.x + r2;
    vertices[3].pos.y = 16.0f + first->pos.y - r2;
    vertices[4].pos.x = 32.0f + first->pos.x + r1;
    vertices[4].pos.y = 16.0f + first->pos.y + r1;
    vertices[5].pos.x = 32.0f + first->pos.x + r2;
    vertices[5].pos.y = 16.0f + first->pos.y + r2;
    vertices[6].pos.x = 32.0f + first->pos.x - r1;
    vertices[6].pos.y = 16.0f + first->pos.y + r1;
    vertices[7].pos.x = 32.0f + first->pos.x - r2;
    vertices[7].pos.y = 16.0f + first->pos.y + r2;
    vertices[8].pos = vertices[0].pos;
    vertices[9].pos = vertices[1].pos;
    for (i = 0; i < 10; i++)
    {
        vertices[i].pos.z = 0.8f;
        vertices[i].w = 1.0f;
        vertices[i].diffuse = 0xff000000;
    }
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 8, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    g_StageManager.dialogueSprites[0].scaleX = -1.5f;
    g_StageManager.dialogueSprites[0].scaleY = -1.75f;
    g_StageManager.dialogueSprites[0].pos.z = 0.7f;
    g_StageManager.dialogueSprites[0].pos.x = 416.0f;
    g_StageManager.dialogueSprites[0].pos.y = 464.0f;
    saved = g_StageManager.dialogueSprites[0].color;
    g_StageManager.dialogueSprites[0].color = 0xffe0c0c0;
    g_SpriteEngine->Draw(&g_StageManager.dialogueSprites[0]);
    g_StageManager.dialogueSprites[0].scaleX = 1.5f;
    g_StageManager.dialogueSprites[0].scaleY = 1.75f;
    g_StageManager.dialogueSprites[0].pos.z = 0.5f;
    g_StageManager.dialogueSprites[0].pos.x = 32.0f;
    g_StageManager.dialogueSprites[0].pos.y = 16.0f;
    g_StageManager.dialogueSprites[0].color = saved;
    g_StageManager.dialogueSprites[1].rotation.z = g_StageManager.dialogueSprites[1].rotation.z * -1.0f;
    g_StageManager.dialogueSprites[1].pos.z = 0.6f;
    saved = g_StageManager.dialogueSprites[1].color;
    g_StageManager.dialogueSprites[1].color = 0xffe0c0c0;
    g_SpriteEngine->DrawAndFlush(&g_StageManager.dialogueSprites[1]);
    g_StageManager.dialogueSprites[1].rotation.z = g_StageManager.dialogueSprites[1].rotation.z * -1.0f;
    g_StageManager.dialogueSprites[1].pos.z = 0.5f;
    g_StageManager.dialogueSprites[1].color = saved;
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
}

void BounceBulletsAtZones(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 posPrev;
    int i;
    int prevZone;
    int zone;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState != 0)
        {
            bullet->scriptFuncState--;
            continue;
        }
        posPrev = bullet->pos - bullet->velocity;
        if (bullet->pos.x > 124.117744f && bullet->pos.x < 259.88226f && bullet->pos.y > 140.11774f && bullet->pos.y < 275.88226f)
        {
            zone = 0;
        }
        else if (bullet->pos.x > 56.23549f && bullet->pos.x < 327.76453f && bullet->pos.y > 72.23549f && bullet->pos.y < 343.76453f)
        {
            zone = 1;
        }
        else
        {
            zone = 2;
        }
        if (posPrev.x > 124.117744f && posPrev.x < 259.88226f && posPrev.y > 140.11774f && posPrev.y < 275.88226f)
        {
            prevZone = 0;
        }
        else if (posPrev.x > 56.23549f && posPrev.x < 327.76453f && posPrev.y > 72.23549f && posPrev.y < 343.76453f)
        {
            prevZone = 1;
        }
        else
        {
            prevZone = 2;
        }
        if (zone != prevZone)
        {
            bullet->scriptFuncState = 2;
            bullet->velocity *= -1.0f;
            if (zone == 0 || prevZone == 0)
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 135.76451f / 67.882256f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 135.76451f / 67.882256f + 208.0f;
            }
            else
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 67.882256f / 135.76451f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 67.882256f / 135.76451f + 208.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, 3.1415927f);
        }
    }
}

void SpawnTwinEffects41(Enemy *enemy, EnemyScriptInstr *instr)
{
    Effect *effect;

    effect = g_EffectManager.SpawnReservedEffect(0x41, &enemy->pos, 9, 1, 0xffffffff);
    effect = g_EffectManager.SpawnReservedEffect(0x41, &enemy->pos, 10, 1, 0xffffffff);
    g_EffectManager.anm->StartScript(&effect->sprite, 0x63);
    g_StageManager.spellCardDrawCallback = DrawTwinEffects;
}

void BounceBulletsAtZones2(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 posPrev;
    int i;
    int prevZone;
    int zone;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState != 0)
        {
            bullet->scriptFuncState--;
            continue;
        }
        posPrev = bullet->pos - bullet->velocity;
        if (bullet->pos.x > 112.80403f && bullet->pos.x < 271.19595f && bullet->pos.y > 128.80403f && bullet->pos.y < 287.19595f)
        {
            zone = 0;
        }
        else if (bullet->pos.x > 33.60807f && bullet->pos.x < 350.39194f && bullet->pos.y > 49.60807f && bullet->pos.y < 366.39194f)
        {
            zone = 1;
        }
        else
        {
            zone = 2;
        }
        if (posPrev.x > 112.80403f && posPrev.x < 271.19595f && posPrev.y > 128.80403f && posPrev.y < 287.19595f)
        {
            prevZone = 0;
        }
        else if (posPrev.x > 33.60807f && posPrev.x < 350.39194f && posPrev.y > 49.60807f && posPrev.y < 366.39194f)
        {
            prevZone = 1;
        }
        else
        {
            prevZone = 2;
        }
        if (zone != prevZone)
        {
            bullet->scriptFuncState = 2;
            bullet->velocity *= -1.0f;
            if (zone == 0 || prevZone == 0)
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 158.39194f / 79.19597f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 158.39194f / 79.19597f + 208.0f;
            }
            else
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 79.19597f / 158.39194f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 79.19597f / 158.39194f + 208.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, 3.1415927f);
        }
    }
}

void EndTwinEffects(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_EffectManager.GetReservedEffect(9)->active = 0;
    g_EffectManager.GetReservedEffect(10)->active = 0;
    g_StageManager.dialogueSpriteCount = 2;
}

void StartResurrection(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.StartNameOnly("「リザレクション」", enemy);
}

void SpawnTwinEffects3A(Enemy *enemy, EnemyScriptInstr *instr)
{
    Effect *effect;

    effect = g_EffectManager.SpawnReservedEffect(0x3a, &enemy->pos, 9, 1, 0xffffffff);
    effect = g_EffectManager.SpawnReservedEffect(0x3a, &enemy->pos, 10, 1, 0xffffffff);
    g_EffectManager.anm->StartScript(&effect->sprite, 0x65);
    g_StageManager.spellCardDrawCallback = DrawTwinEffects;
}

void BounceBulletsAtZones3(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 posPrev;
    int i;
    int prevZone;
    int zone;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState != 0)
        {
            bullet->scriptFuncState--;
            continue;
        }
        posPrev = bullet->pos - bullet->velocity;
        if (bullet->pos.x > 56.23549f && bullet->pos.x < 327.76453f && bullet->pos.y > 88.23549f && bullet->pos.y < 359.76453f)
        {
            zone = 0;
        }
        else if (bullet->pos.x > -32.0f && bullet->pos.x < 416.0f && bullet->pos.y > 0.0f && bullet->pos.y < 448.0f)
        {
            zone = 1;
        }
        else
        {
            zone = 2;
        }
        if (posPrev.x > 56.23549f && posPrev.x < 327.76453f && posPrev.y > 88.23549f && posPrev.y < 359.76453f)
        {
            prevZone = 0;
        }
        else if (posPrev.x > -31.100006f && posPrev.x < 416.0f && posPrev.y > 0.0f && posPrev.y < 448.0f)
        {
            prevZone = 1;
        }
        else
        {
            prevZone = 2;
        }
        if (zone != prevZone)
        {
            bullet->scriptFuncState = 2;
            bullet->velocity *= -1.0f;
            if (zone == 0 || prevZone == 0)
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 224.0f / 135.76451f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 224.0f) * 224.0f / 135.76451f + 224.0f;
            }
            else
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 135.76451f / 224.0f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 224.0f) * 135.76451f / 224.0f + 224.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, 3.1415927f);
        }
    }
}

void KeepFormation(Enemy *enemy, EnemyScriptInstr *instr)
{
    int count;
    int key;
    Enemy *leader;
    Enemy *member;
    float targ;
    float diff;

    key = enemy->currentCtx->vars.ints2[2];
    member = enemy->leader;
    if (member == NULL)
    {
        return;
    }
    count = 0;
    while (member->nextFollower != NULL)
    {
        member = member->nextFollower;
        if (member->currentCtx->vars.ints2[2] == key)
        {
            member->currentCtx->vars.ints2[1] = count;
            if (count == 0)
            {
                leader = member;
            }
            count++;
        }
    }
    enemy->currentCtx->vars.ints[5] = 0;
    if (enemy->currentCtx->vars.ints[6] != count)
    {
        if (enemy->currentCtx->vars.ints[6] != 0)
        {
            enemy->currentCtx->vars.ints[5] = 1;
        }
        enemy->currentCtx->vars.ints[6] = count;
    }
    key = enemy->currentCtx->vars.ints2[1];
    enemy->currentCtx->vars.ints[7]++;
    if (key != 0)
    {
        targ = AddNormalizeAngle(leader->orbitAngle, key * 6.2831855f / count);
        if (leader->currentCtx->vars.ints[7] != enemy->currentCtx->vars.ints[7])
        {
            targ = AddNormalizeAngle(targ, leader->orbitAngularVelocity);
        }
        diff = AddNormalizeAngle(enemy->orbitAngle, enemy->orbitAngularVelocity);
        diff = targ - diff;
        if (fabsf(diff) > 3.1415927f)
        {
            diff = diff > 0.0f ? -6.2831855f + diff : 6.2831855f + diff;
        }
        diff = diff * 0.02f;
        enemy->orbitAngle = AddNormalizeAngle(enemy->orbitAngle, diff);
    }
}

void FlashAndShakeScreen(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 30, 5, 0x40ffffff, 0, 0x15);
    ScreenEffect::StartEffect(7, 4, 120, 190, 60, 0x15);
}

void BeamHitsPlayer160(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 origin(enemy->absPos.x - enemy->currentCtx->vars.floats[0], enemy->absPos.y - enemy->currentCtx->vars.floats[1], 0.0f);
    D3DXVECTOR3 size(590.0f, 160.0f, 0.0f);
    D3DXVECTOR3 size2(590.0f, 128.0f, 0.0f);
    D3DXVECTOR3 center(size.x / 2.0f + origin.x, origin.y, 0.0f);

    if (enemy->callbackTimer.HasTickedEvery(12))
    {
        g_Player.CheckLaserHit(&center, &size2, &origin, enemy->sprite.rotation.z, 1);
    }
    g_Player.CheckLaserHit(&center, &size, &origin, enemy->sprite.rotation.z, 0);
}

void BeamHitsPlayer240(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 origin(enemy->pos.x - enemy->currentCtx->vars.floats[0], enemy->pos.y - enemy->currentCtx->vars.floats[1], 0.0f);
    D3DXVECTOR3 size(590.0f, 240.0f, 0.0f);
    D3DXVECTOR3 size2(590.0f, 192.0f, 0.0f);
    D3DXVECTOR3 center(size.x / 2.0f + origin.x, origin.y, 0.0f);

    if (enemy->callbackTimer.HasTickedEvery(12))
    {
        g_Player.CheckLaserHit(&center, &size2, &origin, enemy->sprite.rotation.z, 1);
    }
    g_Player.CheckLaserHit(&center, &size, &origin, enemy->sprite.rotation.z, 0);
}

void BeamHitsPlayer288(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 origin(enemy->absPos.x - enemy->currentCtx->vars.floats[0], enemy->absPos.y - enemy->currentCtx->vars.floats[1], 0.0f);
    D3DXVECTOR3 size(590.0f, 288.0f, 0.0f);
    D3DXVECTOR3 size2(590.0f, 224.0f, 0.0f);
    D3DXVECTOR3 center(size.x / 2.0f + origin.x, origin.y, 0.0f);

    if (enemy->callbackTimer.HasTickedEvery(12))
    {
        g_Player.CheckLaserHit(&center, &size2, &origin, enemy->sprite.rotation.z, 1);
    }
    g_Player.CheckLaserHit(&center, &size, &origin, enemy->sprite.rotation.z, 0);
}

void SetScreenEffectHoldFrames(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_ScreenEffectHoldFrames = instr->args[1].i;
}

void ToggleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & enemy->currentCtx->vars.ints[0])
        {
            if (bullet->sprites.sprite0.autoRotate == 1)
            {
                bullet->sprites.sprite0.autoRotate = 0;
                bullet->sprites.sprite0.blendMode = 1;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex + 0x10);
                bullet->harmless = 1;
                ((Vector2 *)&bullet->velocity)
                    ->SetPolar(enemy->currentCtx->vars.floats[0], g_App.gameSpeed * enemy->currentCtx->vars.floats[1]);
            }
            else
            {
                bullet->sprites.sprite0.autoRotate = 1;
                bullet->sprites.sprite0.blendMode = 0;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex - 0x10);
                bullet->harmless = 0;
                ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, g_App.gameSpeed * bullet->speed);
            }
        }
    }
    if (enemy->currentCtx->vars.ints[1] == 0)
    {
        Enemy *follower = enemy;

        while (follower->nextFollower != NULL)
        {
            follower = follower->nextFollower;
            follower->halted = 1;
        }
        g_StageManager.dialogueSprites[0].SetPendingInterrupt(2);
        g_StageManager.dialogueSprites[1].SetPendingInterrupt(2);
    }
    else
    {
        Enemy *follower = enemy;

        while (follower->nextFollower != NULL)
        {
            follower = follower->nextFollower;
            follower->halted = 0;
        }
        g_StageManager.dialogueSprites[0].SetPendingInterrupt(1);
        g_StageManager.dialogueSprites[1].SetPendingInterrupt(1);
    }
}

void CycleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & enemy->currentCtx->vars.ints[0])
        {
            if (bullet->sprites.sprite0.autoRotate == 1)
            {
                bullet->sprites.sprite0.autoRotate = 0;
                bullet->sprites.sprite0.blendMode = 1;
                bullet->sprites.sprite0.alpha = 0;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex + 0x10);
                bullet->harmless = 1;
                ((Vector2 *)&bullet->velocity)
                    ->SetPolar(bullet->angle, g_App.gameSpeed * enemy->currentCtx->vars.floats[1]);
            }
            else if (bullet->sprites.sprite0.autoRotate == 0)
            {
                bullet->sprites.sprite0.autoRotate = 2;
                bullet->sprites.sprite0.alpha = 0;
                bullet->sprites.sprite0.SetAlphaInterp(15, 0, 0, 0xff);
            }
            else
            {
                bullet->sprites.sprite0.autoRotate = 1;
                bullet->sprites.sprite0.blendMode = 0;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex - 0x10);
                bullet->harmless = 0;
                ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, g_App.gameSpeed * bullet->speed);
            }
        }
    }
}

void TintStageRed(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_StageManager.SetTintColor(0xffc03030);
}

void ShakeScreenRamp(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(7, 16, 20, 20, 20, 0x15);
}

void MarkFollowersNearBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 diff;
    int i;
    Enemy *member;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & 0x100000)
        {
            member = enemy->nextFollower;
            while (member != NULL)
            {
                if (member->currentCtx->vars.ints2[2] == 0)
                {
                    diff = bullet->pos - member->pos;
                    if (D3DXVec3LengthSq(&diff) < 4096.0f)
                    {
                        member->currentCtx->vars.ints2[2] = 60;
                        member->currentCtx->vars.ints[7] = enemy->currentCtx->vars.ints[7];
                    }
                }
                member = member->nextFollower;
            }
        }
    }
}

void FlashScreen180(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 180, 1, 0xffffffff, 0, 0x15);
}

void SetGameSpeed(Enemy *enemy, EnemyScriptInstr *instr)
{
    int divisor;
    float newSpeed;

    divisor = instr->args[1].i;
    newSpeed = 1.0f / divisor;
    g_App.gameSpeed = newSpeed;
}

void GetPracticeCard(Enemy *enemy, EnemyScriptInstr *instr)
{
    enemy->currentCtx->vars.ints[0] = g_GameManager.practiceSpellCard;
}

void ScriptEndSpellCardName(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.EndSpellCardName();
}

void GetSpellCardsCaptured(Enemy *enemy, EnemyScriptInstr *instr)
{
    enemy->currentCtx->vars.ints[0] = g_GameManager.stats->spellCardsCaptured;
}

void SetFrozen(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.isFrozen = instr->args[1].i;
    if (g_GameManager.isFrozen)
    {
        g_StageManager.dialogueSprites[0].SetPendingInterrupt(2);
        g_StageManager.dialogueSprites[1].SetPendingInterrupt(2);
    }
    else
    {
        g_StageManager.dialogueSprites[0].SetPendingInterrupt(1);
        g_StageManager.dialogueSprites[1].SetPendingInterrupt(1);
    }
}

void SpawnEnemiesAtMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    // Unused.
    D3DXVECTOR3 pos;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & 0x100000)
        {
            enemy->currentCtx->vars.floats[0] = bullet->angle;
            g_EnemyManager.SpawnWithVars(enemy->currentCtx->vars.ints2[2], &bullet->pos, 800, -2, 10,
                                         &enemy->currentCtx->vars);
            bullet->flags &= ~0x100000;
        }
    }
}

void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet;
    int i;

    g_App.gameSpeed = 1.0f / instr->args[1].i;
    g_StageManager.dialogueSprites[0].SetPendingInterrupt(2);
    g_StageManager.dialogueSprites[1].SetPendingInterrupt(2);
    bullet = g_BulletManager.bullets;
    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= g_App.gameSpeed;
        bullet->sprites.sprite0.baseSpriteIndex = bullet->sprites.sprite0.spriteIndex;
        if (bullet->sprites.sprite0.spriteIndex >= 0x60 && bullet->sprites.sprite0.spriteIndex <= 0x6f)
        {
            g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, 0x6f);
        }
    }
}

void EndSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    float scale = 1.0f / g_App.gameSpeed;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= scale;
        if (bullet->sprites.sprite0.spriteIndex >= 0x60 && bullet->sprites.sprite0.spriteIndex <= 0x6f)
        {
            g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.baseSpriteIndex);
        }
    }
    g_App.gameSpeed = 1.0f / instr->args[1].i;
    if (g_App.gameSpeed < 1.0f)
    {
        g_App.stateFlags |= 0x20;
    }
    g_App.gameSpeed = 1.0f;
    g_StageManager.dialogueSprites[0].SetPendingInterrupt(1);
    g_StageManager.dialogueSprites[1].SetPendingInterrupt(1);
}

void DropBombOrLifeItem(Enemy *enemy, EnemyScriptInstr *instr)
{
    if (g_Player.bomb.running)
    {
        g_ItemManager.SpawnItem(&enemy->pos, 3, 0);
    }
    else
    {
        g_ItemManager.SpawnItem(&enemy->pos, 5, 0);
    }
}
