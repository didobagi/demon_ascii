#include <stdlib.h>
#include "../include/types.h"
#include "../include/collision.h"
#include "../include/world.h"

GameObject* collision_check_cell(World *world, int x, int y, EntityType type) {
    GameObject *entities[10];
    int count = world_entity_in_region(world, x, y, x, y, entities, 10);

    for (int i = 0;i < count;i ++) {
        if (entities[i]->entity_type == type && entities[i]->active) {
            return entities[i];
        }
    }
    return NULL;
}

GameObject* collision_check_shape(World *world, GameObject *entity, EntityType type) {
    for (int i = 0;i < entity->shape.point_count;i ++) {
        int point_x = entity->cell_x + entity->shape.rotated_points[i].x;
        int point_y = entity->cell_y + entity->shape.rotated_points[i].y;
        //check collision here
        GameObject *hit = collision_check_cell(world, point_x, point_y, type);
        if (hit && hit != entity) {
            return hit;
        }
    }
    return NULL;
}

bool collision_would_block_movement(World *world, GameObject *entity, int target_x, int target_y) {
    if (!world_is_walkable(world, target_x, target_y)) {
        return true;
    }

    if (entity->shape.point_count > 0) {
        for (int i = 0;i < entity->shape.point_count;i ++) {
            int point_x = target_x + entity->shape.rotated_points[i].x;
            int point_y = target_y + entity->shape.rotated_points[i].y;

            if (!world_is_walkable(world, point_x, point_y)) {
                return true;
            }
        }
    }
    //TODO: entities can overlap 
    //
    return false;
}

bool collision_would_block_movement_ZLTP(World *world, GameObject *entity, int target_x, int target_y,
                                         int y_min, int y_max) {
    if (!world_is_walkable(world, target_x, target_y)) {
        return true;
    }

    if (entity->shape.point_count > 0) {
        for (int i = 0;i < entity->shape.point_count;i ++) {
            int shape_local_y = entity->shape.original_points[i].y;
            //only test points within vertical limits
            //points above ymax are ignored
            if (shape_local_y < y_min || shape_local_y > y_max) {
                continue; //skip this point, its out of test reg
            }
            int point_x = target_x + entity->shape.rotated_points[i].x;
            int point_y = target_y + entity->shape.rotated_points[i].y;
            if (!world_is_walkable(world, point_x, point_y)) {
                return true;
            }
        }
    }
    //TODO: entities can overlap 
    //
    return false;
}

