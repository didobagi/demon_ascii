#ifndef SPAWN_H
#define SPAWN_H

#include "types.h"
#include "world.h"
#include "map_builder.h"
#include "map_template.h"

typedef struct {
    GameObject **enemies; //array of enemy pointers
    int count;
} SpawnResult;


SpawnResult spawn_enemies_in_room(World *world, PlacedRoom *room, Template *templ);

SpawnResult spawn_all_enemies(World *world, MapGenResult *gen_result, TemplateLibrary *library);

#endif
