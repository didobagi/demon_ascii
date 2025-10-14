#include "../include/animation.h"
#include <string.h>

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
    if (entity->animation_total_frames <= 1) {
        return;
    }

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
        for (int i = 0; i < entity->shape.point_count; i++) {
            entity->shape.rotated_points[i] = entity->shape.original_points[i];
        }
    }
}


