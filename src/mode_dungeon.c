#include "../include/mode_dungeon.h"
#include "../include/game_state.h"
#include "../include/map_builder.h"
#include "../include/spawn.h"
#include "../include/movement.h"
#include "../include/movement.h"
#include "../include/animation.h"
#include "../include/enemy_ai.h"
#include "../include/rotation.h"
#include "../include/collision.h"
#include "../include/render.h"
#include "../include/shapes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern FILE *debug_log;

// Helper for spawning system to register enemies
void dungeon_register_enemy(DungeonModeData *data, GameObject *enemy) {
    if (data->enemy_count < 100) {
        data->all_enemies[data->enemy_count] = enemy;
        data->enemy_count++;
    }
}

DungeonModeData* dungeon_mode_create(GameState *game_state) {
    DungeonModeData *data = malloc(sizeof(DungeonModeData));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(DungeonModeData));
    data->game_state = game_state;
    
    // Create world 
    const int WORLD_WIDTH = 400;
    const int WORLD_HEIGHT = 400;
    data->world = create_world(WORLD_WIDTH, WORLD_HEIGHT);
    if (!data->world) {
        free(data);
        return NULL;
    }
    
    // Load templates
    data->template_library = create_template_library();
    Template *circle_room = load_template_from_file("templates/circle_room.txt");
    Template *large_room = load_template_from_file("templates/large_room.txt");
    if (large_room) add_template_to_library(data->template_library, large_room);
    if (circle_room) add_template_to_library(data->template_library, circle_room);
    
    
    // Generate dungeon
    MapGenParams params = {
        .min_rooms = 5,
        .max_rooms = 10,
        .room_spacing = 10,
        .max_placement_attempts = 100,
        .extra_corridor_chance = 0.0f
    };
    data->gen_result = generate_dungeon(data->world, data->template_library, params);
    
    // Spawn enemies
    SpawnResult spawn_results =
    spawn_all_enemies(data->world, &data->gen_result, data->template_library);  
    for (int i = 0;i < spawn_results.count;i ++) {
        dungeon_register_enemy(data, spawn_results.enemies[i]);
    }
    free(spawn_results.enemies);
    
    // Initialize player
    memset(&data->player, 0, sizeof(GameObject));
    data->player.entity_type = ENTITY_PLAYER;
    data->player.active = true;

    int spawn_x, spawn_y;
    if (find_player_spawn_position(data->world, &data->gen_result, 
                data->template_library, &spawn_x, &spawn_y)) {
        data->player.cell_x = spawn_x;
        data->player.cell_y = spawn_y;
    } else {
        // Fallback to first room center if no @ marker found
        if (data->gen_result.room_count > 0) {
            data->player.cell_x = data->gen_result.rooms[0].center_x;
            data->player.cell_y = data->gen_result.rooms[0].center_y;
        } else {
            data->player.cell_x = WORLD_WIDTH / 2;
            data->player.cell_y = WORLD_HEIGHT / 2;
        }
    }

    data->player.v_x = (float)data->player.cell_x;
    data->player.v_y = (float)data->player.cell_y;
    data->player.color = COLOR_RED;
    data->player.shape.texture = TEXTURE_FIRE;
    
    // Setup player shape memory
    static Point player_rotated[100];
    data->player.shape.rotated_points = player_rotated;
    static float player_distances[100];
    data->player.shape.distances = player_distances;
    
    world_add_entity(data->world, data->player.cell_x, data->player.cell_y, &data->player);
    movement_init_entity(&data->player);
    
    // Setup player animation
    data->player.anim_idle_frames = carachter_breathe_frames;
    data->player.anim_idle_frame_counts = carachter_breathe_frame_counts;
    data->player.anim_idle_total_frames = carachter_breathe_total_frames;
    data->player.anim_walk_frames = carachter_breathe_frames;
    data->player.anim_walk_frame_counts = carachter_breathe_frame_counts;
    data->player.anim_walk_total_frames = carachter_breathe_total_frames;
    data->player.current_anim_state = ANIM_STATE_WALK;
    animation_switch_to(&data->player, ANIM_STATE_IDLE, 0.2f);

    data->player.demon_form_template = carachter_breathe_frame0;
    data->player.demon_form_point_count = carachter_breathe_frame0_count;
    data->player.snake_form_template = snake_form_idle_frame0;
    data->player.snake_form_point_count = snake_form_idle_frame0_count;
    data->player.snake_form_idle_frames = snake_form_idle_frames;
    data->player.snake_form_idle_frame_counts = snake_form_idle_frame_counts;
    data->player.snake_form_idle_total_frames = snake_form_idle_total_frames;

    data->player.in_snake_form = false;
    data->player.is_morphing = false;

    for (int i = 0; i < 100; i++) {
        data->player.point_collected[i] = true;
    }
    // Initialize camera
    camera_init(&data->camera, game_state->term_width, game_state->term_height);
    
    data->frame = 0;
    data->moved_this_frame = false;
    
    return data;
}

void dungeon_mode_destroy(DungeonModeData *data) {
    if (!data) return;
    
    // Cleanup enemies (same as your cleanup_enemies)
    for (int i = 0; i < data->enemy_count; i++) {
        if (data->all_enemies[i]) {
            world_remove_entity(data->world, 
                              data->all_enemies[i]->cell_x, 
                              data->all_enemies[i]->cell_y, 
                              data->all_enemies[i]);
            
            if (data->all_enemies[i]->shape.rotated_points) {
                free(data->all_enemies[i]->shape.rotated_points);
            }
            if (data->all_enemies[i]->shape.distances) {
                free(data->all_enemies[i]->shape.distances);
            }
            
            free(data->all_enemies[i]);
        }
    }
    
    // Cleanup world and generation data
    free_map_gen_result(&data->gen_result);
    destroy_template_library(data->template_library);
    destroy_world(data->world);
    
    free(data);
}

void dungeon_handle_player_command(DungeonModeData *data, PlayerCommand cmd) {
    if (cmd == CMD_NONE || data->moved_this_frame) return;
    
    int dx = 0, dy = 0;
    
    switch (cmd) {
        case CMD_MOVE_UP:    dy = -1; break;
        case CMD_MOVE_DOWN:  dy = 1; break;
        case CMD_MOVE_LEFT:  dx = -1; break;
        case CMD_MOVE_RIGHT: dx = 1; break;
        case CMD_MORPH:
            if (!data->player.is_moving) {
                start_morph(&data->player);
            }
            break;
        default: return;
    }

    if (dx != 0) {
        update_entity_facing(&data->player, dx, 0);
    }
    if (movement_try_mov(&data->player, data->world, dx, dy)) {
        data->moved_this_frame = true;

        for (int i = 0; i < data->enemy_count; i++) {
            GameObject *enemy = data->all_enemies[i];
            if (enemy && enemy->active) {
                int dist_x = abs(enemy->cell_x - data->player.cell_x);
                int dist_y = abs(enemy->cell_y - data->player.cell_y);

                if (dist_x <= 4 && dist_y <= 4) {
                    data->player.v_x = (float)data->player.cell_x;
                    data->player.v_y = (float)data->player.cell_y;
                    data->player.is_moving = false;

                    data->state = DUNGEON_STATE_TRANSITIONING;
                    data->transition_timer = 1.5f;
                    data->transition_target = GAME_MODE_DIALOGUE;
                    data->transition_enemy = enemy;
                    return;
                }
            }
        }
    }
}


void dungeon_mode_update(DungeonModeData *data, PlayerCommand cmd, float delta_time) {
    data->frame++;
    data->moved_this_frame = false;

    if (data->state == DUNGEON_STATE_TRANSITIONING) {
        data->transition_timer -= delta_time;

        if (data->transition_timer <= 0.0f) {
            data->game_state->dialogue_enemy = data->transition_enemy;
            data->game_state->dialogue_player = &data->player;
            game_state_transition_to(data->game_state, data->transition_target);
            return;
        }

        return;
    }

    if (cmd == CMD_COMBAT_TEST) {
    data->game_state->dialogue_enemy = data->enemy_count > 0 ? data->all_enemies[0] : NULL;
    data->game_state->dialogue_player = &data->player;
    game_state_transition_to(data->game_state, GAME_MODE_DIALOGUE);
    return;
}
    // Handle player input
    dungeon_handle_player_command(data, cmd);
    
    // Update player
    animation_update(&data->player, delta_time);
    morph_update (&data->player, delta_time);
    movement_update(&data->player, delta_time);
    
    //Update vis after player move
    world_update_visibility(data->world, data->player.cell_x, data->player.cell_y, 31);

    // Update enemies
    // Update enemies
    for (int i = 0; i < data->enemy_count; i++) {
        if (data->all_enemies[i] && data->all_enemies[i]->active) {
            enemy_ai_update(data->all_enemies[i], &data->player, data->world, delta_time);
            animation_update(data->all_enemies[i], delta_time);
            movement_update(data->all_enemies[i], delta_time);

            // Check if enemy moved into player range
            GameObject *enemy = data->all_enemies[i];
            int dist_x = abs(enemy->cell_x - data->player.cell_x);
            int dist_y = abs(enemy->cell_y - data->player.cell_y);

            if (dist_x <= 2 && dist_y <= 2) {
                data->player.v_x = (float)data->player.cell_x;
                data->player.v_y = (float)data->player.cell_y;
                data->player.is_moving = false;

                data->state = DUNGEON_STATE_TRANSITIONING;
                data->transition_timer = 1.5f;
                data->transition_target = GAME_MODE_DIALOGUE;
                data->transition_enemy = enemy;
                return;
            }
        }
    }

    // Update camera
    camera_follow_entity_smooth(&data->camera, &data->player, 
                               data->world->width, data->world->height);
    
    // Check for mode transitions
    // TODO: boss defeated?!? or other transition
    
    // TODO: Check collision with enemy to enter combat
   
}

void dungeon_mode_render(DungeonModeData *data, FrameBuffer *fb) {
    Camera render_camera = data->camera;

    if (data->state == DUNGEON_STATE_TRANSITIONING) {
        int shake_intensity = 13;
        int shake_x = (rand() % (shake_intensity * 2 + 1)) - shake_intensity;
        int shake_y = (rand() % (shake_intensity * 2 + 1)) - shake_intensity;

        render_camera.x += shake_x;
        render_camera.y += shake_y;
    }

    render_world(fb, data->world, &render_camera, data->frame);

    if (data->state == DUNGEON_STATE_TRANSITIONING) {
        int mid_x = data->camera.width / 2;
        int mid_y = data->camera.height / 2;

        char timer_text[32];
        snprintf(timer_text, sizeof(timer_text), "! %.1f !", data->transition_timer);
        draw_text_centered(fb, mid_y, timer_text, COLOR_RED);
    }
    render_world(fb, data->world, &data->camera, data->frame);
}

