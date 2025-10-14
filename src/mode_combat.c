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
    if (cmd != CMD_NONE) {
        game_state_transition_to(data->game_state, GAME_MODE_DUNGEON_EXPLORATION);
    }
}

void combat_mode_render(CombatModeData *data, FrameBuffer *fb) {
    // TODO: Combat rendering
    // For now, just show placeholder text
    init_frame_buffer(fb, 80, 25);
    // Draw "COMBAT MODE" in center
    present_frame(fb);
}
