// The stage background, th07's Stage, from 0x4071a0, as th07's
// stage.cpp.

#include <string.h>

#include "effect.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

Stage::Stage()
{
    memset(this, 0, sizeof(Stage));
    this->camera.eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    this->camera.at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    this->camera.up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    this->camera.fov = 0.5235988f;
    this->cameraFrom = this->camera;
    this->cameraTo = this->camera;
}
