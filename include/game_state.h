#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "dialogue.h"
#include "types.h"
#include "frame_buffer.h"
#include "input.h"
#include "controls.h"


typedef struct DungeonModeData DungeonModeData;
typedef struct CombatModeData CombatModeData;
typedef struct DialogueModeData DialogueModeData;
typedef struct ScrollModeData ScrollModeData;

//lifecycles
GameState* game_state_create(int term_width, int term_height); 
void game_state_destroy(GameState *state);

//mode transitions
void game_state_transition_to(GameState *state, GameMode new_mode);
void game_state_update_transition(GameState *state);

//dispatch
void game_state_update(GameState *state, PlayerCommand cmd, float delta_time);
void game_state_render(GameState *state, FrameBuffer *fb);


#endif
