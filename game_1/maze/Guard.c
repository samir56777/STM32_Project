#include "Guard.h"
#include "LCD.h"
#include "Map.h"
#include <math.h>

#define PI 3.14159f

static bool Guard_HasLineOfSight(int x0, int y0, int x1, int y1)
{
    float dx = (float)(x1 - x0);
    float dy = (float)(y1 - y0);

    float steps = fabsf(dx);
    if (fabsf(dy) > steps) {
        steps = fabsf(dy);
    }

    if (steps < 1.0f) {
        return true;
    }

    float xStep = dx / steps;
    float yStep = dy / steps;

    float x = (float)x0;
    float y = (float)y0;

    for (int i = 0; i <= (int)steps; i++) {
        int col = (int)x / TILE_SIZE;
        int row = (int)y / TILE_SIZE;

        if (Map_IsWall(row, col)) {
            return false;
        }

        x += xStep;
        y += yStep;
    }

    return true;
}

static void Guard_DrawBlockedRay(int x0, int y0, float angleDeg, int range)
{
    float rad = angleDeg * PI / 180.0f;
    float dx = cosf(rad);
    float dy = sinf(rad);

    float x = (float)x0;
    float y = (float)y0;

    for (int i = 0; i < range; i++) {
        int col = (int)x / TILE_SIZE;
        int row = (int)y / TILE_SIZE;

        if (Map_IsWall(row, col)) {
            break;
        }

        x += dx;
        y += dy;
    }

    LCD_Draw_Line(x0, y0, (int)x, (int)y, 10);
}


void Guard_Init(Guard_t* g, int16_t x, int16_t y)
{
    g->x = x;
    g->y = y;
    g->size = 8;

    g->angle = 0.0f;   // facing right
    g->speed = 4.0f;   // degrees per update

    g->range = 120;
    g->fov = 60;
}

void Guard_Update(Guard_t* g)
{   if (g->x < 0 || g->y < 0) {
    return;
}

    g->angle += g->speed;

    if (g->angle >= 360.0f) {
        g->angle -= 360.0f;
    }
}

void Guard_Draw(Guard_t* g)
{   if (g->x < 0 || g->y < 0) {
    return;
}

    int cx = g->x + g->size / 2;
    int cy = g->y + g->size / 2;

    // draw guard body
    LCD_Draw_Rect(g->x, g->y, g->size, g->size, 31, 1);

    // draw cone-like vision with rays, blocked by walls
    for (int a = -(g->fov / 2); a <= (g->fov / 2); a += 15) {
        Guard_DrawBlockedRay(cx, cy, g->angle + (float)a, g->range);
    }
}

bool Guard_CanSeePlayer(Guard_t* g,
                        int16_t px, int16_t py,
                        int16_t pw, int16_t ph)
{   
    if (g->x < 0 || g->y < 0) {
    return false;
}

    int gcx = g->x + g->size / 2;
    int gcy = g->y + g->size / 2;

    int pcx = px + pw / 2;
    int pcy = py + ph / 2;

    float dx = (float)(pcx - gcx);
    float dy = (float)(pcy - gcy);

    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > g->range) {
        return false;
    }

    float playerAngle = atan2f(dy, dx) * 180.0f / PI;
    float diff = playerAngle - g->angle;

    if (diff < -180.0f) diff += 360.0f;
    if (diff > 180.0f)  diff -= 360.0f;
    if (diff < 0) diff = -diff;

    if (diff > (g->fov / 2)) {
        return false;
    }

    if (!Guard_HasLineOfSight(gcx, gcy, pcx, pcy)) {
        return false;
    }

    return true;
}