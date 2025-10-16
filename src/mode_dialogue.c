#include "../include/mode_dialogue.h"
#include "../include/render.h"
#include "../include/game_state.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations for internal functions
static DialogueFragment* generate_initial_fragment(DialogueModeData *data);
static void handle_choice_selection(DialogueModeData *data, int choice_index);
static uint32_t sample_current_conditions(GameObject *player, GameObject *enemy);

DialogueModeData* dialogue_mode_create(GameState *game_state, GameObject *enemy, GameObject *player) {
    DialogueModeData *data = malloc(sizeof(DialogueModeData));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(DialogueModeData));
    data->game_state = game_state;
    data->enemy = enemy;
    data->player = player;
    
    // Load dialogue library (in real game this would be loaded once at startup)
    DialogueLibrary *library = create_dialogue_library();
    DialogueFragmentPool *pool = load_dialogue_pool_from_file(
        "dialogue_fragments/bonobo_dialogue.txt", 
        "bonobo"
    );
    
    if (pool && library) {
        add_pool_to_library(library, pool);
        
        // Get pool for this enemy type
        DialogueFragmentPool *enemy_pool = get_dialogue_pool_for_entity(library, enemy->entity_type);
        if (enemy_pool) {
            data->fragment_pool = enemy_pool->fragments;
            data->fragment_pool_size = enemy_pool->count;
        }
    }
    
    // TODO: Store library reference so we can clean it up
    // For now it leaks, but we'll fix architecture later
    
    // Generate initial fragment based on current state
    data->current_fragment = generate_initial_fragment(data);
    data->selected_choice = 0;
    
    return data;
}

void dialogue_mode_destroy(DialogueModeData *data) {
    if (!data) return;
    
    // Don't free fragment_pool - it's owned by library
    // TODO: Add library cleanup when we store library reference
    
    free(data);
}

void dialogue_mode_update(DialogueModeData *data, PlayerCommand cmd) {
    if (!data->current_fragment) return;
    
    int choice_count = data->current_fragment->choice_count;
    if (choice_count == 0) return;
    
    switch (cmd) {
        case CMD_MOVE_UP:
            data->selected_choice--;
            if (data->selected_choice < 0) {
                data->selected_choice = choice_count - 1;
            }
            break;
            
        case CMD_MOVE_DOWN:
            data->selected_choice++;
            if (data->selected_choice >= choice_count) {
                data->selected_choice = 0;
            }
            break;
            
        case CMD_MORPH:  // Space bar confirms choice
            handle_choice_selection(data, data->selected_choice);
            break;
            
        default:
            break;
    }
}

void dialogue_mode_render(DialogueModeData *data, FrameBuffer *fb) {
    init_frame_buffer(fb, data->game_state->term_width, data->game_state->term_height);
    
    if (!data->current_fragment) {
        draw_text_centered(fb, data->game_state->term_height / 2, 
                          "No dialogue available", COLOR_RED);
        present_frame(fb);
        return;
    }
    
    int mid_y = data->game_state->term_height / 2;
    int start_y = mid_y - 8;
    
    // Draw dialogue box border
    draw_text_centered(fb, start_y, "================================", COLOR_BRIGHT_BLACK);
    draw_text_centered(fb, start_y + 1, "   ENCOUNTER", COLOR_YELLOW);
    draw_text_centered(fb, start_y + 2, "================================", COLOR_BRIGHT_BLACK);
    
    // Draw dialogue text - simple version for now
    draw_text_centered(fb, start_y + 4, data->current_fragment->text, COLOR_WHITE);
    
    // Draw choices
    int choice_y = start_y + 7;
    for (int i = 0; i < data->current_fragment->choice_count; i++) {
        Color color = (i == data->selected_choice) ? COLOR_YELLOW : COLOR_BRIGHT_BLACK;
        char choice_text[80];
        snprintf(choice_text, sizeof(choice_text), "%s %s", 
                (i == data->selected_choice) ? ">" : " ",
                data->current_fragment->choices[i].text);
        draw_text_centered(fb, choice_y + i, choice_text, color);
    }
    
    draw_text_centered(fb, start_y + 12, "================================", COLOR_BRIGHT_BLACK);
    draw_text_centered(fb, start_y + 13, "Arrow keys to select, SPACE to confirm", COLOR_BRIGHT_BLACK);
    
    present_frame(fb);
}

// Internal helper functions
static DialogueFragment* generate_initial_fragment(DialogueModeData *data) {
    if (!data->fragment_pool || data->fragment_pool_size == 0) {
        return NULL;
    }
    
    // Sample current game state
    uint32_t conditions = sample_current_conditions(data->player, data->enemy);
    
    // Find all fragments that match current conditions
    DialogueFragment *valid_fragments[10];
    int valid_count = 0;
    
    for (int i = 0; i < data->fragment_pool_size; i++) {
        DialogueFragment *frag = &data->fragment_pool[i];
        
        // Check if fragment's conditions are met
        // Fragment matches if all its required conditions are present
        if (frag->condition_flags == COND_NONE || 
            (frag->condition_flags & conditions) == frag->condition_flags) {
            valid_fragments[valid_count] = frag;
            valid_count++;
            if (valid_count >= 10) break;
        }
    }
    
    // Pick one randomly from valid fragments
    if (valid_count > 0) {
        int chosen = rand() % valid_count;
        return valid_fragments[chosen];
    }
    
    // Fallback to first fragment if none match
    return &data->fragment_pool[0];
}

static void handle_choice_selection(DialogueModeData *data, int choice_index) {
    DialogueChoice *choice = &data->current_fragment->choices[choice_index];

    if (choice->is_endpoint) {
        data->game_state->dialogue_result.active = true;

        switch (choice->outcome.type) {
            case OUTCOME_END_COMBAT:
                data->game_state->dialogue_result.target_mode = GAME_MODE_TURN_BASED_COMBAT;
                data->game_state->start_transition = true;
                break;
            case OUTCOME_END_PUZZLE:
                data->game_state->dialogue_result.target_mode = GAME_MODE_DUNGEON_EXPLORATION;
                game_state_transition_to(data->game_state, data->game_state->dialogue_result.target_mode);
                break;
            case OUTCOME_END_DEXTERITY:
                data->game_state->dialogue_result.target_mode = GAME_MODE_DUNGEON_EXPLORATION;
                game_state_transition_to(data->game_state, data->game_state->dialogue_result.target_mode);
                break;
            case OUTCOME_END_PEACEFUL:
                data->game_state->dialogue_result.target_mode = GAME_MODE_DUNGEON_EXPLORATION;
                game_state_transition_to(data->game_state, data->game_state->dialogue_result.target_mode);
                break;
            default:
                game_state_transition_to(data->game_state, GAME_MODE_DUNGEON_EXPLORATION);
                break;
        } 
    } else {
        // Continue to next fragment - find it by ID
        for (int i = 0; i < data->fragment_pool_size; i++) {
            if (data->fragment_pool[i].id == choice->next_fragment_id) {
                data->current_fragment = &data->fragment_pool[i];
                data->selected_choice = 0;  // Reset selection
                return;
            }
        }
    }
}

static uint32_t sample_current_conditions(GameObject *player, GameObject *enemy) {
    uint32_t conditions = COND_NONE;
    
    // Check player form
    if (player->in_snake_form) {
        conditions |= COND_PLAYER_SNAKE_FORM;
    } else {
        conditions |= COND_PLAYER_DEMON_FORM;
    }
    
    // TODO: Add health checks when health system exists
    // For now assume medium health
    
    // Check enemy AI state
    if (enemy->ai_state == AI_STATE_FLEE) {
        conditions |= COND_ENEMY_FLEEING;
    } else if (enemy->ai_state == AI_STATE_PURSUE) {
        conditions |= COND_ENEMY_PURUSE;
    }
    
    // Mark as first encounter for now
    conditions |= COND_FIRST_ENCOUNTER;
    
    return conditions;
}
