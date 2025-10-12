#include "../include/map_builder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool rooms_overlap(PlacedRoom *room_a, PlacedRoom *room_b, int spacing) {
    int a_left = room_a->x - spacing;
    int a_right = room_a->x + room_a->width + spacing;
    int a_top = room_a->y - spacing;
    int a_bottom = room_a->y + room_a->height + spacing;

    int b_left = room_b->x - spacing;
    int b_right = room_b->x + room_b->width + spacing;
    int b_top = room_b->y - spacing;
    int b_bottom = room_b->y + room_b->height + spacing;

    bool x_overlap = a_left < b_right && b_left < a_right;
    bool y_overlap = a_top < b_bottom && b_top < a_bottom;

    return x_overlap && y_overlap;
}

void stamp_template_to_world(World *world, Template *template, int x, int y) {
    for (int ty = 0; ty < template->height; ty++) {
        for (int tx = 0; tx < template->width; tx++) {
            int world_x = x + tx;
            int world_y = y + ty;
            
            if (world_x < 0 || world_x >= world->width ||
                world_y < 0 || world_y >= world->height) {
                continue;
            }
            
            int template_index = ty * template->width + tx;
            TerrainType terrain = template->terrain[template_index];
            
            if (terrain == TERRAIN_OPTIONAL_WALL) {
                TerrainType existing = world_get_terrain(world, world_x, world_y);
                if (existing != TERRAIN_FLOOR) {
                    continue;
                }
                terrain = TERRAIN_WALL;
            }
            
            world_set_terrain(world, world_x, world_y, terrain);
        }
    }
}

static bool try_place_room(World *world, Template *template, 
                          PlacedRoom *placed_rooms, int placed_count,
                          MapGenParams params, PlacedRoom *out_room) {
    for (int attempt = 0; attempt < params.max_placement_attempts; attempt++) {
        int max_x = world->width - template->width;
        int max_y = world->height - template->height;
        
        if (max_x <= 0 || max_y <= 0) {
            return false;
        }
        
        int x = rand() % max_x;
        int y = rand() % max_y;
        PlacedRoom candidate;
        candidate.x = x;
        candidate.y = y;
        candidate.width = template->width;
        candidate.height = template->height;
        candidate.center_x = x + template->width / 2;
        candidate.center_y = y + template->height / 2;
        strncpy(candidate.name, template->name, sizeof(candidate.name) - 1);
        candidate.name[sizeof(candidate.name) - 1] = '\0';
        
        bool overlaps = false;
        for (int i = 0; i < placed_count; i++) {
            if (rooms_overlap(&candidate, &placed_rooms[i], params.room_spacing)) {
                overlaps = true;
                break;
            }
        }
        
        if (!overlaps) {
            *out_room = candidate;
            return true;
        }
    }
    
    return false;
}

static void carve_corridor_seg(World *world, int x1, int y1, int x2, int y2, int width) {
    bool is_horizontal = (y1 == y2);

    if (is_horizontal) {
        if (x1 > x2) {
            int temp = x1;
            x1 = x2;
            x2 = temp;
        }

        int half_w = width / 2;
        int y_start = y1 - half_w;
        int y_end = y1 + half_w;

        for (int x = x1;x <= x2;x ++) {
            for (int y = y_start;y <= y_end;y ++) {
                world_set_terrain(world, x, y, TERRAIN_FLOOR);
            }
        }
    } else {
        if (y1 > y2) {
            int temp = y1;
            y1 = y2;
            y2 = temp;
        }

        int half_w = width / 2;
        int x_start = x1 - half_w;
        int x_end = x1 + half_w;

        for (int y = y1;y <= y2;y ++) {
            for (int x = x_start;x <= x_end;x ++) {
                world_set_terrain(world, x, y, TERRAIN_FLOOR);
            }
        }
    }
}

static void carve_l_corridor (World *world, PlacedRoom *room_a, PlacedRoom *room_b, int width) {
    int x1 =room_a->center_x;
    int y1 =room_a->center_y;
    int x2 =room_b->center_x;
    int y2 =room_b->center_y;
    
    if (rand() % 2 == 0) {
        carve_corridor_seg(world, x1, y1, x2, y1, width);
        carve_corridor_seg(world, x2, y1, x2, y2, width);
    } else {
        carve_corridor_seg(world, x1, y1, x1, y2, width);
        carve_corridor_seg(world, x1, y2, x2, y2, width);
    }
}

static void generate_corridors (World *world, PlacedRoom *rooms, int room_count,
                                int corridor_w, float extra_corridor_chance) {
    if (room_count < 2) {
        return;
    }

    bool *connected = malloc(sizeof(bool) * room_count);
    for (int i = 0;i < room_count;i ++) {
        connected[i] = false;
    }
    connected[0] = true; //setting root

    
    int connected_count = 1;
    //build the tree
    while (connected_count < room_count) {
        int unconnected_room = -1;
        for (int i = 0;i < room_count;i ++) {
            if (!connected[i]) {
                unconnected_room = i;
                break;
            }
        }
        int connected_room = -1;
        int attempts = 0;
        while (attempts < 100) {
            int candidate = rand() % room_count;
            if (connected[candidate]) {
                connected_room = candidate;
                break;
            }
            attempts ++;
        }
        if (connected_room == -1) {
            connected_room = 0;
        }
        
        carve_l_corridor(world, &rooms[connected_room], &rooms[unconnected_room], corridor_w);
        connected[unconnected_room] = true;
        connected_count++;

        }
    if (extra_corridor_chance > 0.0f) {
        for (int i = 0;i < room_count;i ++) {
            for (int j = i + 1;j < room_count;j ++) { //implicit guarding
                float roll_da_dice = (float)rand() / (float)RAND_MAX;
                if (roll_da_dice < extra_corridor_chance) {
                    carve_l_corridor(world, &rooms[i], &rooms[j], corridor_w);
                }
            }
        }
    }
    free(connected);
}

MapGenResult generate_dungeon(World *world, TemplateLibrary *library, MapGenParams params) {

    extern FILE *debug_log;

    MapGenResult result;
    result.success = false;
    result.room_count = 0;

    result.rooms = malloc(sizeof(PlacedRoom) * params.max_rooms);
    if (!result.rooms) {
        return result;
    }
    if (debug_log) {
        fprintf(debug_log, "Generating dungeon with %d-%d rooms...\n", params.min_rooms, params.max_rooms);
        fflush(debug_log);
    }

    int target_rooms = params.min_rooms;
    if (params.max_rooms > params.min_rooms) {
        target_rooms = params.min_rooms + rand() % (params.max_rooms - params.min_rooms + 1);
    }

    for (int i = 0;i < target_rooms;i ++) {
        Template *templ =get_random_template(library);
        if (!templ) {
            if (debug_log) {fprintf(debug_log, "NO TEMPLATES AVAILABLE!!!");
                fflush(debug_log);}
            break;
        }

        PlacedRoom placed_room;
        if (try_place_room(world, templ, result.rooms, result.room_count,
                           params, &placed_room)) {
            
            stamp_template_to_world(world, templ, placed_room.x, placed_room.y);

            result.rooms[result.room_count] = placed_room;
            result.room_count++;

            if(debug_log){ fprintf(debug_log,"Placed room %d: %s at (%d,%d)\n", 
                    result.room_count, placed_room.name, placed_room.x, placed_room.y);
            fflush(debug_log); }
        } else {
            if (debug_log) {fprintf(debug_log,"Failed to place room %d after %d attempts\n", 
                    i + 1, params.max_placement_attempts);
            fflush(debug_log); }
        }

    }

    if (result.room_count >= params.min_rooms) {
        result.success = true;
        printf("succesfully placed %d rooms\n", result.room_count);
    } else {
        printf("Failed to meet minimum room requirement (%d < %d)\n", 
                result.room_count, params.min_rooms);
    }

    if (result.room_count >= 2) {
        generate_corridors(world, result.rooms, result.room_count,
                           3, params.extra_corridor_chance);
    }
    return result;
}

void free_map_gen_result (MapGenResult *result) {
    if (result && result->rooms) {
        free(result->rooms);
        result->rooms = NULL;
        result->room_count = 0;
    }
}
