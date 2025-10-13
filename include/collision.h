#ifndef COLLISION_H
#define COLLISION_H

#include "types.h"

GameObject* collision_check_cell (World *world, int x, int y, EntityType type);
GameObject* collision_check_shape (World *world, GameObject *entity, EntityType type);
bool collision_would_block_movement (World *world, GameObject *entity, int x, int y);
bool collision_would_block_movement_ZLTP (World *world, GameObject *entity, int target_x, int target_y,
                                          int y_min, int y_max);

#endif
