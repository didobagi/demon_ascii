#include "../include/world.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


World* create_world (int width, int height) {
    World *world = malloc(sizeof(World));
    if (!world) {
        return NULL;
    }

    world->width = width;
    world->height = height;

    int total_cells = width * height;
    world->grid = malloc(sizeof(Cell)*total_cells);
    if (!world->grid) {
        free(world);
        return NULL;
    }
    //basic fog
    world->visibility = malloc(sizeof(bool) * total_cells);
    if (!world->visibility) {
        free(world->grid);
        free(world);
        return NULL;
    }

    for (int i = 0;i < total_cells;i ++) {
        world->grid[i].terrain = TERRAIN_WALL;
        world->grid[i].entities = NULL;
        world->grid[i].entity_capacity = 0;
        world->grid[i].entity_count = 0;
        world->visibility[i] = false;
    }
    return world;
}

void destroy_world (World *world) {
    if (!world) {
        return;
    }

    int total_cells = world->width * world->height;
    for (int i = 0;i < total_cells;i ++) {
        if (world->grid[i].entities) {
            free(world->grid[i].entities);
            //free array, not entity thems!!
        }
    }
    free(world->grid);
    free(world->visibility);
    free(world);
}

void world_set_terrain (World *world, int x, int y, TerrainType terrain) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return;
    }
    int index = y * world->width + x;
    world->grid[index].terrain = terrain;
}

TerrainType world_get_terrain (World *world, int x, int y) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return TERRAIN_WALL;
    }
    int index = y * world->width + x;
    return world->grid[index].terrain;
}

bool world_is_walkable (World *world, int x, int y) {
    TerrainType terrain = world_get_terrain(world, x, y);
    return terrain != TERRAIN_WALL && terrain != TERRAIN_WATER;
}

void world_add_entity (World *world, int x,int y, GameObject *entity) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return;
    }
    int index = y * world->width + x;
    Cell *cell = &world->grid[index];

    if (cell->entity_count >= cell->entity_capacity) {
        int new_capacity;
        if (cell->entity_capacity == 0) {
            new_capacity = 4;
        } else {
            new_capacity = cell->entity_capacity * 2;
        }

        GameObject **new_array = realloc(cell->entities,
                                         sizeof(GameObject)*new_capacity);
                                //realloc with NULL pointer = malloc?
        if (!new_array) {
            return;
        }

        cell->entities = new_array;
        cell->entity_capacity = new_capacity;
    }

    cell->entities[cell->entity_count] = entity;
    cell->entity_count ++;
}

void world_remove_entity (World *world, int x, int y, GameObject *entity) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return;
    }
    int index = y * world->width + x;
    Cell *cell = &world->grid[index];

    for (int i = 0;i < cell->entity_count;i ++) {
        if (cell->entities[i] == entity) {
            //swap  
            cell->entity_count--;
            cell->entities[i] =  cell->entities[cell->entity_count];
            
            //if cell now empty was large free the array?
            return;
        }
    }

    //add error logging?
}

int world_entity_in_region (World *world, int min_x,int min_y, int max_x,
                            int max_y, GameObject **buffer, int max_count) {
    int fount_count = 0;
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= world->width) max_x = world->width - 1;
    if (max_y >= world->height) max_y = world->height - 1;

    for (int y = min_y;y <= max_y;y ++) {
        for (int x = min_x;x <= max_x;x ++) {
            int index = y * world->width + x;
            Cell *cell = &world->grid[index];

            for (int i = 0;i < cell->entity_count;i ++) {
                if (fount_count >= max_count) {
                    return fount_count;
                }

                buffer[fount_count] = cell->entities[i];
                fount_count ++;
            }
        }
    }
    return fount_count;
}

static void mark_visible(World *world, int x, int y) {
    if (x >= 0 && x < world->width && y >= 0 && y < world->height) {
        world->visibility[y * world->width + x] = true;
    }
}

static bool is_opaque(World *world, int x, int y) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return true;
    }
    TerrainType terrain = world_get_terrain(world, x, y);
    return terrain == TERRAIN_WALL;
}

static void cast_light(World *world, int cx, int cy, int row,
                      float start, float end, int radius,
                      int xx, int xy, int yx, int yy) {
    if (start < end) return;
    
    int radius_squared = radius * radius;
    
    for (int j = row; j <= radius; j++) {
        int dx = -j - 1;
        int dy = -j;
        bool blocked = false;
        
        while (dx <= 0) {
            dx++;
            
            int X = cx + dx * xx + dy * xy;
            int Y = cy + dx * yx + dy * yy;
            
            float l_slope = (dx - 0.5f) / (dy + 0.5f);
            float r_slope = (dx + 0.5f) / (dy - 0.5f);
            
            if (start < r_slope) {
                continue;
            }
            else if (end > l_slope) {
                break;
            }
            
            if (dx * dx + dy * dy < radius_squared) {
                mark_visible(world, X, Y);
            }
            
            if (blocked) {
                if (is_opaque(world, X, Y)) {
                    start = r_slope;
                    continue;
                }
                else {
                    blocked = false;
                }
            }
            else {
                if (is_opaque(world, X, Y) && j < radius) {
                    blocked = true;
                    cast_light(world, cx, cy, j + 1, start, l_slope, radius,
                              xx, xy, yx, yy);
                    start = r_slope;
                }
            }
        }
        
        if (blocked) {
            break;
        }
    }
}

void world_update_visibility(World *world, int viewer_x, int viewer_y, int radius) {
    // Store viewer info for gradient rendering
    world->viewer_x = viewer_x;
    world->viewer_y = viewer_y;
    world->view_radius = radius;
    
    // Clear all visibility
    memset(world->visibility, 0, world->width * world->height * sizeof(bool));
    
    // Mark viewer position
    mark_visible(world, viewer_x, viewer_y);
    
    // 8 octants for shadowcasting
    int multipliers[8][4] = {
        {1,  0,  0,  1},
        {0,  1,  1,  0},
        {0, -1,  1,  0},
        {-1, 0,  0,  1},
        {-1, 0,  0, -1},
        {0, -1, -1,  0},
        {0,  1, -1,  0},
        {1,  0,  0, -1}
    };
    
    for (int oct = 0; oct < 8; oct++) {
        cast_light(world, viewer_x, viewer_y, 1, 1.0f, 0.0f, radius,
                  multipliers[oct][0], multipliers[oct][1],
                  multipliers[oct][2], multipliers[oct][3]);
    }
}

bool world_is_visible(World *world, int x, int y) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return false;
    }
    int index = y * world->width + x;
    return world->visibility[index];
}

float world_get_distance_from_viewer(World *world, int x, int y, int viewer_x, int viewer_y) {
    int dx = x - viewer_x;
    int dy = y - viewer_y;
    return sqrt((float)(dx * dx + dy * dy));
}
