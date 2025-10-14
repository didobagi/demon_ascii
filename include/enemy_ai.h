#ifndef ENEMY_AI_H
#define ENEMY_AI_H

#include "types.h"
#include "world.h"

void enemy_ai_update(GameObject *enemy, GameObject *player, World *world, float delta_time);
bool enemy_try_move_toward(GameObject *enemy, int target_x, int target_y, World *world);
bool enemy_try_flee_from(GameObject *enemy, int target_x, int target_y, World *world);
bool enemy_try_wander(GameObject *enemy, World *world);

#endif
