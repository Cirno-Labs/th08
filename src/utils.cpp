// The angle helpers, 0x43edb0 to 0x43ee90, as th06's utils.cpp: split
// from unsorted.cpp at a plausible seam (nothing in the binary shows it).

#include <math.h>
#include "unsorted.h"

float AddNormalizeAngle(float a, float b)
{
    int i = 0;

    a += b;
    while (a > 3.1415927f)
    {
        a -= 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    while (a < -3.1415927f)
    {
        a += 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    return a;
}

void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);

    out->x = c * in->x - s * in->y;
    out->y = c * in->y + s * in->x;
}
