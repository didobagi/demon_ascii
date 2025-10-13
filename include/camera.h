#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"
#include <stdbool.h>

void camera_init(Camera *camera, int viewport_width, int viewport_height);
void camera_follow_entity(Camera *camera, GameObject *entity, int world_width, int world_height);
void camera_world_to_screen(Camera *camera, int world_x, int world_y, int *screen_x, int *screen_y);
void camera_screen_to_world(Camera *camera, int screen_x, int screen_y, int *world_x, int *world_y);
bool camera_is_visible(Camera *camera, int world_x, int world_y);
void camera_follow_entity_smooth (Camera *camera, GameObject *entity, int world_width, int world_height);
#endif
