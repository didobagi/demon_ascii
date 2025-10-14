#include "../include/game_state.h"
#include "../include/mode_scroll.h"
#include <stdlib.h>
#include <string.h>

ScrollModeData* scroll_mode_create(GameState *game_state) {
    ScrollModeData *data = malloc(sizeof(ScrollModeData));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(ScrollModeData));
    data->game_state = game_state;
    
    // TODO: Initialize scroll
    
    return data;
}

void scroll_mode_destroy(ScrollModeData *data) {
    if (!data) return;
    // TODO: Cleanup scroll
    free(data);
}

void scroll_mode_update(ScrollModeData *data, PlayerCommand cmd, float delta_time) {
    // TODO: Scroll logic
    
    // For now, pressing any key exits scroll back to dungeon
    if (cmd != CMD_NONE) {
        game_state_transition_to(data->game_state, GAME_MODE_DUNGEON_EXPLORATION);
    }
}

void scroll_mode_render(ScrollModeData *data, FrameBuffer *fb) {
    // TODO: Scroll rendering
    // For now, just show placeholder text
    init_frame_buffer(fb, 80, 25);
    // Draw "COMBAT MODE" in center
    present_frame(fb);
}

