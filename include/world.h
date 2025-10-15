#ifndef WORLD_H
#define WORLD_H

#include "types.h"

World* create_world (int width, int height);
void destroy_world (World *world);
void world_set_terrain (World *world, int x, int y, TerrainType terrain);
TerrainType world_get_terrain (World *world, int x, int y);
bool world_is_walkable (World *world, int x, int y);

void world_add_entity (World *world, int x,int y, GameObject *entity);
void world_remove_entity (World *world, int x, int y, GameObject *entity);
int world_entity_in_region (World *world, int min_x,int min_y, int max_x,
                            int max_y, GameObject **buffer, int max_count);
void world_update_visibility (World *world, int viewer_x, int viewer_y, int radius);
bool world_is_visible (World *world, int x, int y);
#endif
