#include "../include/game_state.h"
#include "../include/mode_dungeon.h"
#include "../include/mode_combat.h"
#include "../include/mode_scroll.h"
#include "../include/types.h"
#include <stdlib.h>
#include <string.h>

GameState* game_state_create(int term_width, int term_height) {
    GameState *state = malloc(sizeof(GameState));
    if (!state) return NULL;
    
    memset(state, 0, sizeof(GameState));
    
    state->current_mode = GAME_MODE_DUNGEON_EXPLORATION;
    state->next_mode = GAME_MODE_DUNGEON_EXPLORATION;
    state->mode_changed = false;
    
    state->term_width = term_width;
    state->term_height = term_height;
    
    state->current_level = 1;
    state->player_max_health = 100;
    state->player_current_health = 100;
    
    // Initialize dungeon mode (starting mode)
    state->dungeon_data = dungeon_mode_create(state);
    
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
}

void game_state_update(GameState *state, PlayerCommand cmd, float delta_time) {
    // Dispatch to active mode
    switch (state->current_mode) {
        case GAME_MODE_DUNGEON_EXPLORATION:
            dungeon_mode_update(state->dungeon_data, cmd, delta_time);
            break;
        case GAME_MODE_TURN_BASED_COMBAT:
            combat_mode_update(state->combat_data, cmd);
            break;
        case GAME_MODE_SIDE_SCROLL:
            scroll_mode_update(state->scroll_data, cmd, delta_time);
            break;
        case GAME_MODE_QUIT:
            // Nothing to update
            break;
    }
}

void game_state_render(GameState *state, FrameBuffer *fb) {
    // Dispatch to active mode
    switch (state->current_mode) {
        case GAME_MODE_DUNGEON_EXPLORATION:
            dungeon_mode_render(state->dungeon_data, fb);
            break;
        case GAME_MODE_TURN_BASED_COMBAT:
            combat_mode_render(state->combat_data, fb);
            break;
        case GAME_MODE_SIDE_SCROLL:
            scroll_mode_render(state->scroll_data, fb);
            break;
        case GAME_MODE_QUIT:
            // Nothing to render
            break;
    }
}

