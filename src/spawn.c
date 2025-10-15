#include "../include/spawn.h"
#include "../include/enemy_ai.h"
#include "../include/animation.h"
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

    enemy->anim_idle_frames = bonobo_idle_frames;
    enemy->anim_idle_frame_counts = bonobo_idle_frame_counts;
    enemy->anim_idle_total_frames = bonobo_idle_total_frames;
    enemy->anim_walk_frames = bonobo_walk_frames;
    enemy->anim_walk_frame_counts = bonobo_walk_frame_counts;
    enemy->anim_walk_total_frames = bonobo_walk_total_frames;
    enemy->shape.texture = TEXTURE_SOLID;


    enemy->shape.rotated_points = malloc(sizeof(Point) * 100);
    enemy->shape.distances = malloc(sizeof(float) * 100);

    if (!enemy->shape.rotated_points || !enemy->shape.distances) {
        if (enemy->shape.rotated_points) free(enemy->shape.rotated_points);
        if (enemy->shape.distances) free(enemy->shape.distances);
        free(enemy);
        return NULL;
    }

    enemy->current_anim_state = ANIM_STATE_WALK;
    animation_switch_to(enemy, ANIM_STATE_IDLE, 0.3f);

    for (int i = 0; i < enemy->shape.point_count; i++) {
        enemy->shape.rotated_points[i] = enemy->shape.original_points[i];

        float x = enemy->shape.original_points[i].x;
        float y = enemy->shape.original_points[i].y;
        enemy->shape.distances[i] = sqrt(x*x + y*y);
    }
    enemy->ai_state = AI_STATE_IDLE;
    enemy->alert_timer = 0.0f;

    return enemy;
}

SpawnResult spawn_enemies_in_room(World *world, PlacedRoom *room, Template *template) {
    GameObject **enemies = malloc(sizeof(GameObject*) * 50);  // Max 50 per room
    int count = 0;

    if (!enemies) {
        SpawnResult empty = {NULL, 0};
        return empty;
    }
    
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
                    enemies[count] = enemy;
                    count++;

                    if (count >= 50) {
                        break;
                    }
                }
            }
        }
        if (count >= 50) break;
    }
    SpawnResult result = {enemies, count};
    return result;
}

SpawnResult spawn_all_enemies(World *world, MapGenResult *gen_result, TemplateLibrary *library) {
     GameObject **all_enemies = malloc(sizeof(GameObject*) * 500); //so many enemies better then callback
    int total_count = 0;
    
        if (!all_enemies) {
        SpawnResult empty = {NULL, 0};
        return empty;
    }

    for (int i = 0; i < gen_result->room_count; i++) {
        PlacedRoom *room = &gen_result->rooms[i];
        
        Template *template = get_template_by_name(library, room->name);
        
        if (template) {
            SpawnResult room_result = spawn_enemies_in_room(world, room, template);
            
            for (int j = 0;j < room_result.count;j ++) {
                if (total_count > 500) {
                    free(room_result.enemies);
                    break;
                }
                all_enemies[total_count] = room_result.enemies[j];
                total_count++;
            }
            free(room_result.enemies);
        }
    }
    
    extern FILE *debug_log;
    if (debug_log) {
        fprintf(debug_log, "Spawned %d enemies across %d rooms\n", 
                total_count, gen_result->room_count);
        fflush(debug_log);
    }

    SpawnResult result = {all_enemies, total_count};
    return result;
}
