#include "../include/render.h"
#include "../include/mode_combat.h"
#include "../include/shapes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static CombatCell* get_cell (CombatModeData *data,int x, int y) {
    if (x < 0 || x >= data->grid_width || y < 0 || y >= data->grid_height) {
        return NULL;
    }
    return &data->grid[y * data->grid_width + x];
}

static CombatUnit* get_unit_at (CombatModeData *data, int x, int y) {
    CombatCell *cell = get_cell(data, x, y);
    return cell ? cell->occupant : NULL;
}

CombatModeData* combat_mode_create(GameState *game_state) {
    CombatModeData *data = malloc(sizeof(CombatModeData));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(CombatModeData));
    data->game_state = game_state;
    
    data->grid_width = 15;
    data->grid_height = 10;
    data->grid = malloc(sizeof(CombatCell)* data->grid_height * data->grid_width);

    for (int y = 0;y < data->grid_height;y ++) {
        for (int x = 0;x < data->grid_width;x ++) {
            CombatCell *cell = get_cell(data, x, y);
            cell->walkable = true;
            cell->cover = COVER_NONE;
            cell->occupant = NULL;
        }
    }
    get_cell(data, 5, 3)->cover = COVER_HALF;
    get_cell(data, 5, 4)->cover = COVER_HALF;
    get_cell(data, 2, 8)->cover = COVER_FULL;
    get_cell(data, 2, 7)->cover = COVER_FULL;


    data->player_count = 3;
    for (int i = 0;i < 3;i ++) {
        data->player_units[i] = (CombatUnit){
            .grid_x = 1,
            .grid_y = 1 + i * 3,
            .max_hp = 10,
            .current_hp = 10,
            .move_range = 4,
            .weapon_range = 5,
            .base_hit_chance = 77,
            .is_player_unit = true,
            .has_moved = false,
            .has_acted = false,
            .is_alive = true,
            .display_char = 'Y',
            .color = COLOR_BRIGHT_BLUE
        };
        CombatCell *cell = get_cell(data, data->player_units[i].grid_x,
                data->player_units[i].grid_y);
        cell->occupant = &data->player_units[i];
    }

    data->enemy_count = 3;
    for (int i = 0;i < 3;i ++) {
        data->enemy_units[i] = (CombatUnit){
            .grid_x = 12,
            .grid_y = 2 + i * 3,
            .max_hp = 9,
            .current_hp = 9,
            .move_range = 3,
            .weapon_range = 5,
            .base_hit_chance = 76,
            .is_player_unit = false,
            .has_moved = false,
            .has_acted = false,
            .is_alive = true,
            .display_char = 'E',
            .color = COLOR_BRIGHT_RED,
        };
        CombatCell *cell = get_cell(data, data->enemy_units[i].grid_x,
                data->enemy_units[i].grid_y);
        cell->occupant = &data->enemy_units[i];
    }
    data->player_turn = true;
    data->selected_unit_index = -1;
    data->showing_movement = false;
    data->showing_targets = false;
    data->combat_over = false;

    return data;
}

void combat_mode_destroy(CombatModeData *data) {
    if (!data) return;
    if (data->grid) free(data->grid);
    free(data);
}

void combat_mode_update(CombatModeData *data, PlayerCommand cmd) {
    if (data->combat_over) {
        if (cmd != CMD_NONE) {
            game_state_transition_to(data->game_state, GAME_MODE_DUNGEON_EXPLORATION);
        }
        return;
    }

    //TODO turn logic
}

void combat_mode_render(CombatModeData *data, FrameBuffer *fb) {
    init_frame_buffer(fb, data->game_state->term_width, data->game_state->term_height);
    
    int frame_width = data->grid_width + 2;
    int frame_height = data->grid_height + 2;
    
    int frame_x = (data->game_state->term_width - frame_width) / 2;
    int frame_y = (data->game_state->term_height - frame_height) / 2 - 3;


    int grid_start_x = frame_x + 1;
    int grid_start_y = frame_y + 1;

    int screen_center_x = data->game_state->term_width / 2;
    int screen_center_y = data->game_state->term_height / 2;

    int art_width = 103 - (-2);
    int art_height = 28 - (+5);
    int art_offset_x = -art_width / 2;
    int art_offset_y = -art_height / 2;

    for (int i = 0; i < combat_arena_background_count; i++) {
        int point_x = screen_center_x + combat_arena_background[i].x + art_offset_x;
        int point_y = screen_center_y + combat_arena_background[i].y + art_offset_y;

        buffer_draw_char(fb, point_x, point_y, 'o', COLOR_MAGENTA);
    }

    for (int x = 0; x < frame_width; x++) {
        buffer_draw_char(fb, frame_x + x, frame_y, '-', COLOR_WHITE);
        buffer_draw_char(fb, frame_x + x, frame_y + frame_height - 1, '-', COLOR_WHITE);
    }
    
    for (int y = 0; y < frame_height; y++) {
        buffer_draw_char(fb, frame_x, frame_y + y, '|', COLOR_WHITE);
        buffer_draw_char(fb, frame_x + frame_width - 1, frame_y + y, '|', COLOR_WHITE);
    }
    
    buffer_draw_char(fb, frame_x, frame_y, '+', COLOR_WHITE);
    buffer_draw_char(fb, frame_x + frame_width - 1, frame_y, '+', COLOR_WHITE);
    buffer_draw_char(fb, frame_x, frame_y + frame_height - 1, '+', COLOR_WHITE);
    buffer_draw_char(fb, frame_x + frame_width - 1, frame_y + frame_height - 1, '+', COLOR_WHITE);
    
    for (int y = 0; y < data->grid_height; y++) {
        for (int x = 0; x < data->grid_width; x++) {
            CombatCell *cell = get_cell(data, x, y);
            
            int screen_x = grid_start_x + x;
            int screen_y = grid_start_y + y;
            
            char ch = '.';
            Color color = COLOR_BRIGHT_BLACK;
            
            if (cell->cover == COVER_HALF) {
                ch = '/';
                color = COLOR_YELLOW;
            } else if (cell->cover == COVER_FULL) {
                ch = '#';
                color = COLOR_WHITE;
            }
            
            if (cell->occupant && cell->occupant->is_alive) {
                ch = cell->occupant->display_char;
                color = cell->occupant->color;
            }
            
            buffer_draw_char(fb, screen_x, screen_y, ch, color);
        }
    }
    
    int ui_top_y = frame_y - 2;
    draw_text_centered(fb, ui_top_y, 
                      data->player_turn ? "=== PLAYER TURN ===" : "=== ENEMY TURN ===", 
                      COLOR_YELLOW);
    
    int ui_bottom_y = frame_y + frame_height + 1;
    draw_text_centered(fb, ui_bottom_y,
                      "Arrow keys: select | Space: action | Q: exit", 
                      COLOR_BRIGHT_BLACK);
    
    if (data->selected_unit_index >= 0 && data->selected_unit_index < data->player_count) {
        CombatUnit *unit = &data->player_units[data->selected_unit_index];
        char info[64];
        snprintf(info, sizeof(info), "HP: %d/%d | Move: %d | Range: %d", 
                unit->current_hp, unit->max_hp, unit->move_range, unit->weapon_range);
        draw_text_centered(fb, ui_bottom_y + 1, info, COLOR_CYAN);
    }
    
    present_frame(fb);
}
