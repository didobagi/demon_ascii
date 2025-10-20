#include "../include/mode_dialogue.h"
#include "../include/render.h"
#include "../include/game_state.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static DialogueFragment* generate_initial_fragment(DialogueModeData *data);
static void handle_choice_selection(DialogueModeData *data, int choice_index);
static uint32_t sample_current_conditions(GameObject *player, GameObject *enemy);

static int wrap_text(const char *text, char lines[][300], int max_lines, int max_width) {
    int line_count = 0;
    const char *p = text;
    
    while (*p && line_count < max_lines) {
        // Skip leading spaces
        while (*p == ' ') p++;
        if (!*p) break;
        
        const char *line_start = p;
        const char *last_space = NULL;
        int len = 0;
        
        // Scan ahead
        while (*p && len < max_width) {
            if (*p == ' ') last_space = p;
            p++;
            len++;
        }
        
        // Backtrack to last space if we have more text
        if (*p && last_space) {
            len = last_space - line_start;
            p = last_space + 1;  // Skip the space
        }
        
        // Copy line
        strncpy(lines[line_count], line_start, len);
        lines[line_count][len] = '\0';
        line_count++;
    }
    
    return line_count;
}

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

        DialogueFragmentPool *enemy_pool = get_dialogue_pool_for_entity(library, enemy->entity_type);
        if (enemy_pool) {
            data->fragment_pool = enemy_pool->fragments;
            data->fragment_pool_size = enemy_pool->count;
        }
    }

    data->library = library;    
    
    // Generate initial fragment based on current state
    data->current_fragment = generate_initial_fragment(data);
    data->selected_choice = 0;
    data->typewriter_timer = 0.0f;
    data->chars_revealed = 0;
    
    return data;
}

void dialogue_mode_destroy(DialogueModeData *data) {
    if (!data) return;

    if (data->library) {
        destroy_dialogue_library(data->library);
    } 
    free(data);
}

void dialogue_mode_update(DialogueModeData *data, PlayerCommand cmd) {
    if (!data->current_fragment) return;
    int text_length = strlen(data->current_fragment->text);
    if (data->chars_revealed < text_length) {
        data->typewriter_timer += 0.016f; 
        if (data->typewriter_timer >= 0.01f) { 
            data->typewriter_timer = 0.0f;
            data->chars_revealed++;
        }
        return; // Don't allow input until text fully revealed
    }
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
            
        case CMD_ACTION:  
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
    
    int dialogue_width = (int)(data->game_state->term_width * 0.7f);  
    if (dialogue_width > 90) dialogue_width = 90; 
    if (dialogue_width < 50) dialogue_width = 50;

    int mid_y = data->game_state->term_height / 2;
    int start_y = mid_y - 15;
    
    // Draw dialogue box border
    draw_text_centered(fb, start_y, "================================", COLOR_BRIGHT_BLACK);
    draw_text_centered(fb, start_y + 1, "   ENCOUNTER", COLOR_YELLOW);
    draw_text_centered(fb, start_y + 2, "================================", COLOR_BRIGHT_BLACK);

    char wrapped_lines[3][300];
    int line_count = wrap_text(data->current_fragment->text, wrapped_lines, 3, dialogue_width);

    for (int i = 0; i < line_count; i++) {
        // Apply typewriter effect
        int chars_in_prev_lines = 0;
        for (int j = 0; j < i; j++) {
            chars_in_prev_lines += strlen(wrapped_lines[j]);
        }

        int chars_to_show = data->chars_revealed - chars_in_prev_lines;
        if (chars_to_show < 0) chars_to_show = 0;
        if (chars_to_show > (int)strlen(wrapped_lines[i])) {
            chars_to_show = strlen(wrapped_lines[i]);
        }

        char visible_text[300];
        strncpy(visible_text, wrapped_lines[i], chars_to_show);
        visible_text[chars_to_show] = '\0';

        draw_text_centered(fb, start_y + 4 + i, visible_text, COLOR_WHITE);
    } 
    // Draw choices only if text fully revealed
    int text_length = strlen(data->current_fragment->text);
    if (data->chars_revealed >= text_length) {
        int choice_y = start_y + 7;
        for (int i = 0; i < data->current_fragment->choice_count; i++) {
            Color color = (i == data->selected_choice) ? COLOR_YELLOW : COLOR_BRIGHT_BLACK;

            // Wrap each choice text
            char wrapped_choice_lines[5][300];
            char prefixed_text[300];
            snprintf(prefixed_text, sizeof(prefixed_text), "%s %s",
                    (i == data->selected_choice) ? ">" : " ",
                    data->current_fragment->choices[i].text);

            int choice_line_count = wrap_text(prefixed_text, wrapped_choice_lines, 5, dialogue_width);

            for (int j = 0; j < choice_line_count; j++) {
                draw_text_centered(fb, choice_y, wrapped_choice_lines[j], color);
                choice_y++;
            }
        }
    }
    draw_text_centered(fb, start_y + 15, "================================", COLOR_BRIGHT_BLACK);
    draw_text_centered(fb, start_y + 15, "+++++place holder text++++++++++", COLOR_BRIGHT_BLACK);
    draw_text_centered(fb, start_y + 16, "Arrow keys to select, SPACE to confirm", COLOR_BRIGHT_BLACK);
    
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
                data->chars_revealed = 0;
                data->typewriter_timer = 0.0f;
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
