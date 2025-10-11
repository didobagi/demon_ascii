#ifndef GAME_H
#define GAME_H

#include "types.h"
#include <unistd.h>

int create_object (GameObject objects[], int* count,
                   Point *shape_template, int point_count,
                   float x, float y, float dx, float dy,
                   float collision_radius, TextureType texture,
                   Color color);
void update_transform(GameObject *obj, int max_x, int max_y);
void update_visual(GameObject *obj);
bool check_boundaries(GameObject *obj, int max_x, int max_y);
bool check_obj_collision(GameObject *obj_a, GameObject *obj_b);
void check_collectible_collision (GameObject *player, GameState *game);
void commit_transf(GameObject *obj);
ShapeBounds calculate_shape_bounds (Point *points, int count);
ShapeBounds calculate_shape_bounds_selective (Point *points, int count,
                                    bool *collected, bool in_snake_form);
void bounce (GameObject *obj, int screen_w, int screen_h); 
void spawn_enemy (GameState *game, Point *shape_template, int point_count,
                  float x, float y, float speed); 
void update_enemies (GameState *game);
#endif
