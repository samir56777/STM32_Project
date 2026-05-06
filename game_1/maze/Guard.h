#ifndef GUARD_H
#define GUARD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Guard_s {
    int16_t x;
    int16_t y;
    int16_t size;

    float angle;
    float speed;

    int16_t range;
    int16_t fov;
} Guard_t;

void Guard_Init(Guard_t* g, int16_t x, int16_t y);
void Guard_Update(Guard_t* g);
void Guard_Draw(Guard_t* g);

bool Guard_CanSeePlayer(Guard_t* g,
                        int16_t px, int16_t py,
                        int16_t pw, int16_t ph);

#endif