#ifndef MODE_COMBAT_H
#define MODE_COMBAT_H

#include "types.h"
#include "game_state.h"
#include "frame_buffer.h"
#include "input.h"
#include "controls.h"


typedef struct CombatModeData {
    GameState *game_state;
    // TODO: Add combat-specific data....
} CombatModeData;

CombatModeData* combat_mode_create(GameState *game_state);
void combat_mode_destroy(CombatModeData *data);
void combat_mode_update(CombatModeData *data, PlayerCommand cmd);
void combat_mode_render(CombatModeData *data, FrameBuffer *fb);

#endif
