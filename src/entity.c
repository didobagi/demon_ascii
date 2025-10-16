#include "../include/animation.h"
#include "../include/types.h"
#include "../include/movement.h"
#include "../include/collision.h"
#include "../include/animation.h"
#include "../include/rotation.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define HORIZONTAL_SPEED 1.0f 
#define VERTICAL_SPEED 1.0f
extern FILE *debug_log;

void animation_set(GameObject *entity, 
                   Point **frames, 
                   int *frame_counts, 
                   int total_frames,
                   float speed) {
    entity->animation_frames = frames;
    entity->animation_frame_counts = frame_counts;
    entity->animation_total_frames = total_frames;
    entity->animation_current_frame = 0;
    entity->animation_timer = 0.0f;
    entity->animation_speed = speed;
    
    entity->shape.original_points = frames[0];
    entity->shape.point_count = frame_counts[0];
    //TODO update rotaded points??

}

void animation_update(GameObject *entity, float delta_time) {

    //if (entity->animation_total_frames <= 1) {
    //    return;
    //}

    entity->animation_timer += delta_time;

    if (entity->animation_timer >= entity->animation_speed) {
        entity->animation_timer = 0.0f;

        entity->animation_current_frame++;
        if (entity->animation_current_frame >= entity->animation_total_frames) {
            entity->animation_current_frame = 0;
        }

        entity->shape.original_points = 
            entity->animation_frames[entity->animation_current_frame];
        entity->shape.point_count = 
            entity->animation_frame_counts[entity->animation_current_frame];

        extern void apply_facing_to_shape(GameObject *entity);
        apply_facing_to_shape(entity);
    }
}

void animation_switch_to(GameObject *entity, AnimationState new_state, float speed) {
    if (entity->current_anim_state == new_state) {
        return;
    }
    
    entity->current_anim_state = new_state;
    //entity->animation_timer = 0.0f;
    //entity->animation_current_frame = 0;
    entity->animation_speed = speed;
    
    if (new_state == ANIM_STATE_IDLE) {
        entity->animation_frames = entity->anim_idle_frames;
        entity->animation_frame_counts = entity->anim_idle_frame_counts;
        entity->animation_total_frames = entity->anim_idle_total_frames;    
    } else if (new_state == ANIM_STATE_WALK) {
        entity->animation_frames = entity->anim_walk_frames;
        entity->animation_frame_counts = entity->anim_walk_frame_counts;
        entity->animation_total_frames = entity->anim_walk_total_frames;    
    }
    
    // Update the shape's points to the first frame of new animation
    if (entity->animation_total_frames > 0) {
        entity->shape.original_points = entity->animation_frames[0];
        entity->shape.point_count = entity->animation_frame_counts[0];

        extern void apply_facing_to_shape(GameObject *entity);
        apply_facing_to_shape(entity);
    }
}

void movement_init_entity (GameObject *entity) {
    entity->is_moving = false;
    entity->v_x = (float)entity->cell_x;
    entity->v_y = (float)entity->cell_y;
    entity->target_v_x = entity->v_x;
    entity->target_v_y = entity->v_y;
}

bool movement_try_mov (GameObject *entity, World *world, int dx, int dy) {
    
    int target_x = entity->cell_x + dx;
    int target_y = entity->cell_y + dy;
    
    if (!world_is_walkable(world, target_x, target_y)) {
        return false;
    }

    if (collision_would_block_movement_ZLTP(world, entity, target_x, target_y, 0, 100)) {
        return false;
    }
    
    world_remove_entity(world, entity->cell_x, entity->cell_y, entity);
    

    //if (entity->is_moving) {
    //    entity->v_x = (float)entity->cell_x;
    //    entity->v_y = (float)entity->cell_y;
    //}
    
    entity->cell_x = target_x;
    entity->cell_y = target_y;
    entity->target_v_x = (float)target_x;
    entity->target_v_y = (float)target_y;
    entity->is_moving = true;


    if (entity->entity_type == ENTITY_PLAYER) {
        animation_switch_to(entity, ANIM_STATE_WALK, 0.3f); 
    } else if (entity->entity_type == ENTITY_ENEMY) {
        animation_switch_to(entity, ANIM_STATE_WALK, entity->animation_speed);
    }
    
    if (dx != 0) {
        if (entity->entity_type == ENTITY_PLAYER) {
        entity->move_speed = HORIZONTAL_SPEED;
        }
    } else {
        if (entity->entity_type == ENTITY_PLAYER) {
            entity->move_speed = VERTICAL_SPEED;
        }
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
        if (entity->entity_type == ENTITY_PLAYER) {
            animation_switch_to(entity, ANIM_STATE_IDLE, 0.3f); // Player breathing
        }
        return;
    }

    float move_amount = entity->move_speed;
    if (move_amount > dist) {
        move_amount = dist;
    }
    
    entity->v_x += (dx / dist) * move_amount;
    entity->v_y += (dy / dist) * move_amount;
}

void rotate_shape(Shape *shape, float angle_radians) {
    float cos_angle = cos(angle_radians);
    float sin_angle = sin(angle_radians);

    for (int i = 0;i < shape->point_count;i ++) {
        int orig_x = shape->original_points[i].x;
        int orig_y = shape->original_points[i].y;

        float rotated_x = orig_x * cos_angle - orig_y * sin_angle;
        float rotated_y = orig_x * sin_angle + orig_y * cos_angle;
        
        shape->rotated_points[i].x = (int)(rotated_x + 0.5f); //for rounding
        shape->rotated_points[i].y = (int)(rotated_y + 0.5f);
    }
}

float angle_toward(int from_x, int from_y, int to_x, int to_y) {
    int dx = to_x - from_x;
    int dy = to_y - from_y;
    
    return atan2((float)dy, (float)dx);
}

void update_entity_facing(GameObject *entity, int dx, int dy) {
    if (dx == 0 && dy == 0) {
        return;
    }

    if (dx < 0) {
        // Facing left
        for (int i = 0; i < entity->shape.point_count; i++) {
            entity->shape.rotated_points[i].x = -entity->shape.original_points[i].x;
            entity->shape.rotated_points[i].y = entity->shape.original_points[i].y;
        }
        entity->facing_angle = M_PI;
    } else if (dx > 0) {
        // Facing right
        for (int i = 0; i < entity->shape.point_count; i++) {
            entity->shape.rotated_points[i].x = entity->shape.original_points[i].x;
            entity->shape.rotated_points[i].y = entity->shape.original_points[i].y;
        }
        entity->facing_angle = 0.0f;
    }
    // REMOVE THE ELSE BLOCK! Don't touch facing for vertical movement
    // The entity keeps its previous horizontal facing (left or right)
}

void apply_facing_to_shape(GameObject *entity) {
    float angle = entity->facing_angle;
    
    if (fabs(angle) < 0.01f) {  // Approximately 0
        // Facing right
        for (int i = 0; i < entity->shape.point_count; i++) {
            entity->shape.rotated_points[i] = entity->shape.original_points[i];
        }
    } else if (fabs(angle - M_PI) < 0.01f) {  // Approximately PI
        // Facing left - mirror X only
        for (int i = 0; i < entity->shape.point_count; i++) {
            entity->shape.rotated_points[i].x = -entity->shape.original_points[i].x;
            entity->shape.rotated_points[i].y = entity->shape.original_points[i].y;
        }
    } else {
        // Other angles - rotate
        rotate_shape(&entity->shape, angle);
    }
}
