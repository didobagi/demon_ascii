#include "../include/game_state.h"
#include "../include/mode_dungeon.h"
#include "../include/mode_dialogue.h"
#include "../include/mode_combat.h"
#include "../include/mode_scroll.h"
#include "../include/types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool is_transitioning = false;
static float transition_progress = 0.0f;
static FrameBuffer transition_source;
static FrameBuffer transition_target;
static GameMode transition_to_mode;

static void render_dissolve_transition(GameState *state, FrameBuffer *output) {
    float threshold = transition_progress;
    
    float shake_intensity = (1.0f - transition_progress) * 5.0f;
    int shake_x = (rand() % (int)(shake_intensity * 2 + 1)) - (int)shake_intensity;
    int shake_y = (rand() % (int)(shake_intensity * 2 + 1)) - (int)shake_intensity;
    
    for (int y = 0; y < state->term_height; y++) {
        for (int x = 0; x < state->term_width; x++) {
            //create unique "seed" mult by primes to ensure good distribution
            //same position always generate same seed?
            int seed = x * 73 + y * 137;
            float pixel_threshold = (float)(seed % 1000) / 1000.0f;
            
            int source_x = x + shake_x;
            int source_y = y + shake_y;
            
            if (source_x < 0) source_x = 0;
            if (source_x >= state->term_width) source_x = state->term_width - 1;
            if (source_y < 0) source_y = 0;
            if (source_y >= state->term_height) source_y = state->term_height - 1;
            
            if (pixel_threshold < threshold) {
                output->cells[y][x] = transition_target.cells[source_y][source_x];
                output->colors[y][x] = transition_target.colors[source_y][source_x];
            } else {
                output->cells[y][x] = transition_source.cells[source_y][source_x];
                output->colors[y][x] = transition_source.colors[source_y][source_x];
            }
        }
    }
}

GameState* game_state_create(int term_width, int term_height) {
    GameState *state = malloc(sizeof(GameState));
    if (!state) return NULL;
    
    memset(state, 0, sizeof(GameState));
    
    state->current_mode = GAME_MODE_DUNGEON_EXPLORATION;
    state->next_mode = GAME_MODE_DUNGEON_EXPLORATION;
    state->mode_changed = false;

    state->dialogue_result.active = false;
    state->dialogue_result.target_mode = GAME_MODE_DUNGEON_EXPLORATION;
    state->dialogue_result.difficulty = DIFFICULTY_MEDIUM;
    state->dialogue_result.context_flags = 0;
    
    state->term_width = term_width;
    state->term_height = term_height;
    
    state->current_level = 1;
    state->player_max_health = 100;
    state->player_current_health = 100;
    
    // Initialize dungeon mode (starting mode)
    state->dungeon_data = dungeon_mode_create(state);

    state->start_transition = false;
    return state;
}

void game_state_destroy(GameState *state) {
    if (!state) return;
    
    // Clean up whichever mode is active
    if (state->dungeon_data) {
        dungeon_mode_destroy(state->dungeon_data);
    }
    if (state->combat_data) {
        combat_mode_destroy(state->combat_data);
    }
    if (state->scroll_data) {
        scroll_mode_destroy(state->scroll_data);
    }
    
    free(state);
}

void game_state_transition_to(GameState *state, GameMode new_mode) {
    state->next_mode = new_mode;
    state->mode_changed = true;
}

void game_state_update_transition(GameState *state) {
    if (!state->mode_changed) return;
    
    GameMode old_mode = state->current_mode;
    GameMode new_mode = state->next_mode;
    
    // Cleanup old mode
    switch (old_mode) {
        case GAME_MODE_DUNGEON_EXPLORATION:
            if (state->dungeon_data) {
                dungeon_mode_destroy(state->dungeon_data);
                state->dungeon_data = NULL;
            }
            break;
        case GAME_MODE_DIALOGUE:
            if (state->dialogue_data) {
                dialogue_mode_destroy(state->dialogue_data);
                state->dialogue_data = NULL;
            }
            break;
        case GAME_MODE_TURN_BASED_COMBAT:
            if (state->combat_data) {
                combat_mode_destroy(state->combat_data);
                state->combat_data = NULL;
            }
            break;
        case GAME_MODE_SIDE_SCROLL:
            if (state->scroll_data) {
                scroll_mode_destroy(state->scroll_data);
                state->scroll_data = NULL;
            }
            break;
        default:
            break;
    }
    
    // Initialize new mode
    switch (new_mode) {
        case GAME_MODE_DUNGEON_EXPLORATION:
            state->dungeon_data = dungeon_mode_create(state);
            break;
        case GAME_MODE_DIALOGUE:
            // For testing, pass first enemy from dungeon
            // In real game this would be triggered by collision
            state->dialogue_data = dialogue_mode_create(state, 
                    state->dialogue_enemy, 
                    state->dialogue_player);
            break;
        case GAME_MODE_TURN_BASED_COMBAT:
            state->combat_data = combat_mode_create(state);
            break;
        case GAME_MODE_SIDE_SCROLL:
            state->scroll_data = scroll_mode_create(state);
            break;
        case GAME_MODE_QUIT:
            // Nothing to initialize, just exit
            break;
    }
    
    state->current_mode = new_mode;
    state->mode_changed = false;
    printf("\033[?25l");
    fflush(stdout);
}

void game_state_update(GameState *state, PlayerCommand cmd, float delta_time) {
    if (cmd == CMD_QUIT) {
        state->current_mode = GAME_MODE_QUIT;
        return;
    }
    if (is_transitioning) {
        transition_progress += delta_time * 0.7f;
        
        if (transition_progress >= 1.0f) {
            is_transitioning = false;
            transition_progress = 0.0f;
            game_state_transition_to(state, transition_to_mode);
        }
        return;
    }
    
    if (state->start_transition) {
        is_transitioning = true;
        transition_progress = 0.0f;
        transition_to_mode = state->dialogue_result.target_mode;
        
        init_frame_buffer(&transition_source, state->term_width, state->term_height);
        init_frame_buffer(&transition_target, state->term_width, state->term_height);

        switch (state->current_mode) {
            case GAME_MODE_DIALOGUE:
                dialogue_mode_render(state->dialogue_data, &transition_source);
                break;
            default:
                break;
        }
        
        state->combat_data = combat_mode_create(state);
        combat_mode_render(state->combat_data, &transition_target);
        
        state->start_transition = false;
        return;
    }
    
    if (state->mode_changed) {
        state->mode_changed = false;
        return;
    }

    switch (state->current_mode) {
        case GAME_MODE_DUNGEON_EXPLORATION:
            dungeon_mode_update(state->dungeon_data, cmd, delta_time);
            break;
        case GAME_MODE_DIALOGUE:
            dialogue_mode_update(state->dialogue_data, cmd);
            break;
        case GAME_MODE_TURN_BASED_COMBAT:
            combat_mode_update(state->combat_data, cmd);
            break;
        case GAME_MODE_SIDE_SCROLL:
            scroll_mode_update(state->scroll_data, cmd, delta_time);
            break;
        case GAME_MODE_QUIT:
            break;
    }
}

void game_state_render(GameState *state, FrameBuffer *fb) {
    if (is_transitioning) {
        render_dissolve_transition(state, fb);
        present_frame(fb);
        return;
    }
    
    switch (state->current_mode) {
        case GAME_MODE_DUNGEON_EXPLORATION:
            dungeon_mode_render(state->dungeon_data, fb);
            break;
        case GAME_MODE_DIALOGUE:
            dialogue_mode_render(state->dialogue_data, fb);
            break;
        case GAME_MODE_TURN_BASED_COMBAT:
            combat_mode_render(state->combat_data, fb);
            break;
        case GAME_MODE_SIDE_SCROLL:
            scroll_mode_render(state->scroll_data, fb);
            break;
        case GAME_MODE_QUIT:
            break;
    }
    present_frame(fb);
}
