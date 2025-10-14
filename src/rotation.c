#include "../include/rotation.h"
#include <iso646.h>
#include <math.h>

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
