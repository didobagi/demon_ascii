#ifndef SPAWN_H
#define SPAWN_H

#include "types.h"
#include "world.h"
#include "map_builder.h"

int spawn_enemies_in_room(World *world, PlacedRoom *room, Template *templ);

void spawn_all_enemies(World *world, MapGenResult *gen_result, TemplateLibrary *library);

#endif
