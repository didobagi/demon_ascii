#include "../include/render.h"
#include "../include/mode_combat.h"
#include <stdlib.h>
#include <string.h>

CombatModeData* combat_mode_create(GameState *game_state) {
    CombatModeData *data = malloc(sizeof(CombatModeData));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(CombatModeData));
    data->game_state = game_state;
    
    // TODO: Initialize combat
    
    return data;
}

void combat_mode_destroy(CombatModeData *data) {
    if (!data) return;
    // TODO: Cleanup combat
    free(data);
}

void combat_mode_update(CombatModeData *data, PlayerCommand cmd) {
    // TODO: Combat logic
    
    // For now, pressing any key exits combat back to dungeon
    if (cmd == CMD_MORPH) {
        game_state_transition_to(data->game_state, GAME_MODE_DUNGEON_EXPLORATION);
    }
}

void combat_mode_render(CombatModeData *data, FrameBuffer *fb) {
    init_frame_buffer(fb, data->game_state->term_width, data->game_state->term_height);

    int mid_y = data->game_state->term_height / 2;

    draw_text_centered(fb, mid_y - 3, "================================", COLOR_RED);
    draw_text_centered(fb, mid_y - 2, "       combat mode  ???         ", COLOR_BRIGHT_RED);
    draw_text_centered(fb, mid_y - 1, "================================", COLOR_RED);
    
    draw_text_centered(fb, mid_y + 1, "time to fight? maybe time to talk, maybe both", COLOR_YELLOW);
    
    draw_text_centered(fb, mid_y + 3, "Press space to return to dungeon", COLOR_BRIGHT_BLACK);
    
    present_frame(fb);
}
