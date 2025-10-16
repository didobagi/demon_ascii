#ifndef MODE_SCROLL_H
#define MODE_SCROLL_H


#include "types.h"
#include "frame_buffer.h"
#include "input.h"
#include "game_state.h"


typedef struct ScrollModeData {
    GameState *game_state;
    // TODO: Add scroll-specific data....
} ScrollModeData;

ScrollModeData* scroll_mode_create(GameState *game_state);
void scroll_mode_destroy(ScrollModeData *data);
void scroll_mode_update(ScrollModeData *data, PlayerCommand cmd, float delta_time);
void scroll_mode_render(ScrollModeData *data, FrameBuffer *fb);

#endif
