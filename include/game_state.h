#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "types.h"
#include "frame_buffer.h"
#include "input.h"
#include "controls.h"

typedef enum {
    GAME_MODE_DUNGEON_EXPLORATION,
    GAME_MODE_TURN_BASED_COMBAT,
    GAME_MODE_SIDE_SCROLL,
    GAME_MODE_QUIT
} GameMode;

typedef struct DungeonModeData DungeonModeData;
typedef struct CombatModeData CombatModeData;
typedef struct ScrollModeData ScrollModeData;


typedef struct {
    GameMode current_mode;
    GameMode next_mode; //for transition??
    bool mode_changed;

    DungeonModeData *dungeon_data;
    CombatModeData *combat_data;
    ScrollModeData *scroll_data;

    int current_level;
    int player_max_health;
    int player_current_health;

    int term_width;
    int term_height;
} GameState;

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
