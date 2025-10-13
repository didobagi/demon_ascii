#include "../include/movement.h"
#include "../include/collision.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define HORIZONTAL_SPEED 1.0f 
#define VERTICAL_SPEED 1.0f

void movement_init_entity (GameObject *entity) {
    entity->is_moving = false;
    entity->v_x = (float)entity->cell_x;
    entity->v_y = (float)entity->cell_y;
    entity->target_v_x = entity->v_x;
    entity->target_v_y = entity->v_y;
}

bool movement_try_mov (GameObject *entity, World *world, int dx, int dy) {
    extern FILE *debug_log;
    
    int target_x = entity->cell_x + dx;
    int target_y = entity->cell_y + dy;
    
    if (!world_is_walkable(world, target_x, target_y)) {
        return false;
    }

    if (collision_would_block_movement_ZLTP(world, entity, target_x, target_y, 0, 100)) {
        return false;
    }
    
    world_remove_entity(world, entity->cell_x, entity->cell_y, entity);
    
    if (debug_log && dx != 0) {  // Only log horizontal movement
        fprintf(debug_log, "MOVE: dx=%d | cell_x: %d->%d | v_x: %.2f | is_moving: %d\n",
                dx, entity->cell_x, target_x, entity->v_x, entity->is_moving);
        fflush(debug_log);
    }
    
    //if (entity->is_moving) {
    //    entity->v_x = (float)entity->cell_x;
    //    entity->v_y = (float)entity->cell_y;
    //}
    
    entity->cell_x = target_x;
    entity->cell_y = target_y;
    entity->target_v_x = (float)target_x;
    entity->target_v_y = (float)target_y;
    entity->is_moving = true;
    
    if (dx != 0) {
        entity->move_speed = HORIZONTAL_SPEED;
    } else {
        entity->move_speed = VERTICAL_SPEED;
    }
    
    world_add_entity(world, entity->cell_x, entity->cell_y, entity);
    return true;
}

void movement_update (GameObject *entity, float delta_time) {
    if (!entity->is_moving) {
        return;
    }
    
    float dx = entity->target_v_x - entity->v_x;
    float dy = entity->target_v_y - entity->v_y;
    float dist = sqrt(dx*dx + dy*dy);
    
    if (dist < 0.01f) {
        entity->v_x = entity->target_v_x;
        entity->v_y = entity->target_v_y;
        entity->is_moving = false;
        return;
    }
    
    float move_amount = entity->move_speed;
    if (move_amount > dist) {
        move_amount = dist;
    }
    
    entity->v_x += (dx / dist) * move_amount;
    entity->v_y += (dy / dist) * move_amount;
}
