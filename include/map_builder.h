#ifndef MAP_BUILDER_H
#define MAP_BUILDER_H

#include "world.h"
#include "map_template.h"
#include <stdbool.h>

typedef struct {
    GameObject **enemies; //array of enemy pointers
    int count;
} SpawnResult;


typedef struct {
    int x, y;
    int width, height;
    int center_x, center_y;
    char name[64];
} PlacedRoom;

typedef struct {
    int min_rooms;
    int max_rooms;
    int room_spacing;
    int max_placement_attempts;
    float extra_corridor_chance;
} MapGenParams;

typedef struct {
    PlacedRoom *rooms;
    int room_count;
    bool success;
} MapGenResult;

MapGenResult generate_dungeon(World *world, TemplateLibrary *library, MapGenParams params);

bool rooms_overlap(PlacedRoom *room_a, PlacedRoom *room_b, int spacing);
void stamp_template_to_world(World *world, Template *templ, int x, int y);
void carve_corridor_seg(World *world, int x1, int y1, int x2, int y2, int width);
void free_map_gen_result (MapGenResult *result);

SpawnResult spawn_enemies_in_room(World *world, PlacedRoom *room, Template *templ);

SpawnResult spawn_all_enemies(World *world, MapGenResult *gen_result, TemplateLibrary *library);
bool find_player_spawn_position(World *world, MapGenResult *gen_result, 
                                TemplateLibrary *library, int *out_x, int *out_y);
#endif
