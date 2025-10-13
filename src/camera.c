#include "../include/camera.h"

void camera_init(Camera *camera, int viewport_width, int viewport_height) {
    camera->x = 0;
    camera->y = 0;
    camera->width = viewport_width;
    camera->height = viewport_height;
}

void camera_follow_entity(Camera *camera, GameObject *entity, int world_width, int world_height) {
    camera->x = entity->cell_x - camera->width / 2;
    camera->y = entity->cell_y - camera->height / 2;
    
    // Clamp camera to world boundaries
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
