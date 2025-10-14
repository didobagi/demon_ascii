#include "../include/spawn.h"
#include "../include/enemy_ai.h"
#include "../include/types.h"
#include "../include/shapes.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static GameObject* create_enemy(int world_x, int world_y) {
    GameObject *enemy = malloc(sizeof(GameObject));
    if (!enemy) {
        return NULL;
    }

    memset(enemy, 0, sizeof(GameObject));

    enemy->entity_type = ENTITY_ENEMY;
    enemy->active = true;

    enemy->cell_x = world_x;
    enemy->cell_y = world_y;
    enemy->v_x = (float)world_x;
    enemy->v_y = (float)world_y;

    enemy->ai_state = AI_STATE_IDLE;
    enemy->alert_timer = 0.0f;
    enemy->wander_timer = ((float)rand() / RAND_MAX) * WANDER_MOVE_INTERVAL;
    enemy->move_timer = ((float)rand() / RAND_MAX) * 0.03f;

    enemy->is_moving = false;
    enemy->target_v_x = enemy->v_x;
    enemy->target_v_y = enemy->v_y;
    enemy->move_speed = 1.0f;

    enemy->color = COLOR_YELLOW;

    enemy->shape.original_points = monkey_template;
    enemy->shape.point_count = monkey_point_count;
    enemy->shape.texture = TEXTURE_SOLID;

    enemy->shape.rotated_points = malloc(sizeof(Point) * enemy->shape.point_count);
    enemy->shape.distances = malloc(sizeof(float) * enemy->shape.point_count);

    if (!enemy->shape.rotated_points || !enemy->shape.distances) {
        if (enemy->shape.rotated_points) free(enemy->shape.rotated_points);
        if (enemy->shape.distances) free(enemy->shape.distances);
        free(enemy);
        return NULL;
    }

    for (int i = 0; i < enemy->shape.point_count; i++) {
        enemy->shape.rotated_points[i] = enemy->shape.original_points[i];

        float x = enemy->shape.original_points[i].x;
        float y = enemy->shape.original_points[i].y;
        enemy->shape.distances[i] = sqrt(x*x + y*y);
    }
    enemy->ai_state = AI_STATE_IDLE;
    enemy->alert_timer = 0.0f;

    //reg each enemy
    extern void register_enemy(GameObject *enemy);
    register_enemy(enemy);
    return enemy;
}

int spawn_enemies_in_room(World *world, PlacedRoom *room, Template *template) {
    int spawned_count = 0;
    
    for (int ty = 0; ty < template->height; ty++) {
        for (int tx = 0; tx < template->width; tx++) {
            int template_index = ty * template->width + tx;
            MarkerType marker = template->markers[template_index];
            
            if (marker == MARKER_ENEMY_SPAWN) {
                int world_x = room->x + tx;
                int world_y = room->y + ty;
                
                GameObject *enemy = create_enemy(world_x, world_y);
                if (enemy) {
                    world_add_entity(world, world_x, world_y, enemy);
                    spawned_count++;
                }
            }
        }
    }
    
    return spawned_count;
}

void spawn_all_enemies(World *world, MapGenResult *gen_result, TemplateLibrary *library) {
    int total_spawned = 0;
    
    for (int i = 0; i < gen_result->room_count; i++) {
        PlacedRoom *room = &gen_result->rooms[i];
        
        Template *template = get_template_by_name(library, room->name);
        
        if (template) {
            int spawned = spawn_enemies_in_room(world, room, template);
            total_spawned += spawned;
        }
    }
    
    extern FILE *debug_log;
    if (debug_log) {
        fprintf(debug_log, "Spawned %d enemies across %d rooms\n", 
                total_spawned, gen_result->room_count);
        fflush(debug_log);
    }
}
