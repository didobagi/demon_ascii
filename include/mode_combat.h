#ifndef MODE_COMBAT_H
#define MODE_COMBAT_H

#include "types.h"
#include "game_state.h"
#include "frame_buffer.h"
#include "input.h"

typedef enum {
    COVER_NONE = 0,
    COVER_HALF = 1,
    COVER_FULL = 2,
} CoverType;

typedef struct {
    int grid_x;
    int grid_y;
    float v_x;
    float v_y;
    bool is_moving;
    int max_hp;
    int current_hp;
    int move_range;
    int weapon_range;
    int base_hit_chance;
    bool is_player_unit;
    bool has_moved;
    bool has_acted;
    bool is_alive;

    char display_char; //visual
    Color color;
} CombatUnit;

typedef struct {
    bool walkable;
    CoverType cover;
    CombatUnit *occupant; 
} CombatCell;

typedef struct CombatModeData {
    GameState *game_state;

    int grid_width;
    int grid_height;
    CombatCell *grid;

    CombatUnit player_units[3];
    CombatUnit enemy_units[3];
    int player_count;
    int enemy_count;

    bool player_turn;
    int selected_unit_index;
    bool showing_movement;
    bool showing_targets;

    bool animating_shot;
    float shot_animation_timer;
    int shot_from_x;
    int shot_from_y;
    int shot_to_x;
    int shot_to_y;
    bool shot_hit;

    bool combat_over;
    bool player_won;

    bool *reachable_tiles;
    int reachable_count;
    
    int cursor_x;
    int cursor_y;

    int selected_target_index;
} CombatModeData;


CombatModeData* combat_mode_create(GameState *game_state);
void combat_mode_destroy(CombatModeData *data);
void combat_mode_update(CombatModeData *data, PlayerCommand cmd, float delta_time);
void combat_mode_render(CombatModeData *data, FrameBuffer *fb);

#endif
