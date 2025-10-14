#include "../include/animation.h"
#include "../include/types.h"
#include <stdio.h>

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
        extern FILE *debug_log;

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

    // ADD THIS DEBUG LOG:
    if (debug_log) {
        fprintf(debug_log, "ANIM: entity_type=%d frame=%d points=%d\n", 
                entity->entity_type, entity->animation_current_frame, 
                entity->animation_frame_counts[entity->animation_current_frame]);
        fflush(debug_log);
    }

        entity->shape.original_points = 
            entity->animation_frames[entity->animation_current_frame];
        entity->shape.point_count = 
            entity->animation_frame_counts[entity->animation_current_frame];
        for (int i = 0; i < entity->shape.point_count; i++) {
            entity->shape.rotated_points[i] = entity->shape.original_points[i];
        }
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
        for (int i = 0; i < entity->shape.point_count; i++) {
            entity->shape.rotated_points[i] = entity->shape.original_points[i];
        }
    }
}
