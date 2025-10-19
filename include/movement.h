#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "types.h"
#include "world.h"
#include <stdbool.h>

void movement_init_entity (GameObject *entity);
bool movement_try_mov (GameObject *entity, World *world, int dx, int dy);
void movement_update (GameObject *entity, float delta_time);

void start_morph (GameObject *entity);
void prepare_morph_to_snake (GameObject *entity);
void morph_update (GameObject *entity, float delta_time);
#endif

