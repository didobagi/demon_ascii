#include "../include/camera.h"
#include <math.h>
#include <stdlib.h>

#define CAMERA_LERP_SPEED 0.5f

void camera_init(Camera *camera, int viewport_width, int viewport_height) {
    camera->x_float = 0.0f;
    camera->y_float = 0.0f;
    camera->x = 0;
    camera->y = 0;
    camera->width = viewport_width;
    camera->height = viewport_height;
}

void camera_follow_entity_smooth (Camera *camera, GameObject *entity, 
                                  int world_width, int world_height) {
    // Calculate where camera should be (centered on player)
    float desired_x = entity->v_x - camera->width / 2.0f;
    float desired_y = entity->v_y - camera->height / 2.0f;
    
    // Clamp to world boundaries
    if (desired_x < 0) desired_x = 0;
    if (desired_y < 0) desired_y = 0;
    if (desired_x + camera->width > world_width) {
        desired_x = world_width - camera->width;
    }
    if (desired_y + camera->height > world_height) {
        desired_y = world_height - camera->height;
    }
    
    // Smoothly interpolate camera toward desired position
    float dx = desired_x - camera->x_float;
    float dy = desired_y - camera->y_float;
    
    camera->x_float += dx * CAMERA_LERP_SPEED;
    camera->y_float += dy * CAMERA_LERP_SPEED;
    
    // Snap when very close to avoid infinite creep
    if (fabs(dx) < 0.1f) camera->x_float = desired_x;
    if (fabs(dy) < 0.1f) camera->y_float = desired_y;
    
    // Convert to integers for rendering
    camera->x = (int)(camera->x_float + 0.5);
    camera->y = (int)(camera->y_float + 0.5);
}

void camera_follow_entity (Camera *camera, GameObject *entity, int world_width, int world_height) {
    camera->x = (int)entity->v_x - camera->width / 2;
    camera->y = (int)entity->v_y - camera->height / 2;
    
    if (camera->x < 0) camera->x = 0;
    if (camera->y < 0) camera->y = 0;
    if (camera->x + camera->width > world_width) {
        camera->x = world_width - camera->width;
    }
    if (camera->y + camera->height > world_height) {
        camera->y = world_height - camera->height;
    }
}

void camera_world_to_screen(Camera *camera, int world_x, int world_y, int *screen_x, int *screen_y) {
    *screen_x = world_x - camera->x;
    *screen_y = world_y - camera->y;
}

void camera_screen_to_world(Camera *camera, int screen_x, int screen_y, int *world_x, int *world_y) {
    *world_x = screen_x + camera->x;
    *world_y = screen_y + camera->y;
}

bool camera_is_visible(Camera *camera, int world_x, int world_y) {
    return world_x >= camera->x && 
           world_x < camera->x + camera->width &&
           world_y >= camera->y && 
           world_y < camera->y + camera->height;
}
