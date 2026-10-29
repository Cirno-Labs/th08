// The player, from 0x449ca0, as the Player half of th07's player.cpp.
// The shots follow in their own object.

#include <math.h>
#include <string.h>

#include "effect.h"
#include "player.h"
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
