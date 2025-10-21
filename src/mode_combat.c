#include "../include/render.h"
#include "../include/mode_combat.h"
#include "../include/movement.h"
#include "../include/shapes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void update_unit_visual_position(CombatUnit *unit, float delta_time) {
    if (unit->is_moving) {
        float speed = 10.0f;
        float dx = (float)unit->grid_x - unit->v_x;
        float dy = (float)unit->grid_y - unit->v_y;
        
        if (dx * dx + dy * dy < 0.01f) {
            unit->v_x = (float)unit->grid_x;
            unit->v_y = (float)unit->grid_y;
            unit->is_moving = false;
        } else {
            unit->v_x += dx * speed * delta_time;
            unit->v_y += dy * speed * delta_time;
        }
    }
}

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

static void calculate_reachable_tiles (CombatModeData *data, CombatUnit *unit) {
    if (!data->reachable_tiles) {
        data->reachable_tiles = malloc(sizeof(bool) * data->grid_width * data->grid_height);
    }
    for (int i = 0;i < data->grid_width * data->grid_height;i ++) {
        data->reachable_tiles[i] = false;
    }
    data->reachable_count = 0;

    //flood fill bfs algo
    //exploring tiles layer by layer tracking distance from start
    int *queue = malloc(sizeof(int) * data->grid_height * data-> grid_width * 3);
    int queue_start = 0;
    int queue_end = 0;
    
    queue[queue_end++] = unit->grid_x;
    queue[queue_end++] = unit->grid_y;
    queue[queue_end++] = 0;

    int start_index = unit->grid_y * data->grid_width + unit->grid_x;
    data->reachable_tiles[start_index] = true;
    data->reachable_count++;

    while (queue_start < queue_end) {
        int current_x = queue[queue_start++];
        int current_y = queue[queue_start++];
        int current_distance = queue[queue_start++];

        if (current_distance >= unit->move_range) {
            continue;
        }

        //check all 4 adj tiles
        int directions[4][2] = {
            {0, -1},
            {0, 1},
            {-1, 0},
            {1, 0}
        };
        for (int dir = 0;dir < 4;dir ++) {
            int next_x = current_x + directions[dir][0];
            int next_y = current_y + directions[dir][1];

            if (next_x < 0 || next_x >= data->grid_width ||
                next_y < 0 || next_y >= data->grid_height) {
                continue;
            }
            CombatCell *neighbor_cell = get_cell(data, next_x, next_y);
            if (!neighbor_cell->walkable) {
                continue;
            }
            if (neighbor_cell->occupant != NULL) {
                continue;
            }

            int neighbor_index = next_y * data->grid_width + next_x;
            if (data->reachable_tiles[neighbor_index]) {
                continue;
            }

            data->reachable_tiles[neighbor_index] = true;
            data->reachable_count++;

            queue[queue_end++] = next_x;
            queue[queue_end++] = next_y;
            queue[queue_end++] = current_distance + 1;
        }
    }
    free(queue);
}

static bool can_shoot_target (CombatModeData *data, CombatUnit *shooter, CombatUnit *target) {
    int dx = abs(shooter->grid_x - target->grid_x);
    int dy = abs(shooter->grid_y - target->grid_y);
    int distance = (dx > dy) ? dx : dy;

    if (distance > shooter->weapon_range) {
        return false;
    }

    return true;
}

static int calculate_hit_chance (CombatModeData *data, CombatUnit *shooter, CombatUnit *target) {
    int base_chance = shooter->base_hit_chance;

    int dx = abs(shooter->grid_x - target->grid_x);
    int dy = abs(shooter->grid_y - target->grid_y);

    int distance = (dx > dy) ? dx : dy;
    int distance_penalty = (distance -3) * 5;
    if (distance_penalty < 0) distance_penalty = 0;

        CombatCell *target_cell = get_cell(data, target->grid_x, target->grid_y);
    int cover_bonus = 0;
    if (target_cell) {
        if (target_cell->cover == COVER_HALF) cover_bonus = 15;
        if (target_cell->cover == COVER_FULL) cover_bonus = 30;
    }
    
    int final_chance = base_chance - distance_penalty - cover_bonus;
    if (final_chance < 10) final_chance = 10;
    if (final_chance > 95) final_chance = 95;
    
    return final_chance;
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

    CombatCell *cover1 = get_cell(data, 5 , 3);
    cover1->cover = COVER_HALF;
    cover1->walkable = false;
    CombatCell *cover2 = get_cell(data, 5 , 4);
    cover2->cover = COVER_HALF;
    cover2->walkable = false;
    CombatCell *cover3 = get_cell(data, 2 , 7);
    cover3->cover = COVER_FULL;
    cover3->walkable = false;
    CombatCell *cover4 = get_cell(data, 2 , 8);
    cover4->cover = COVER_FULL;
    cover4->walkable = false;

    data->player_count = 3;
    for (int i = 0;i < 3;i ++) {
        data->player_units[i] = (CombatUnit){
            .grid_x = 1,
            .grid_y = 1 + i * 3,
            .v_x = 1.0f,
            .v_y = (float)(1 + i * 3),
            .is_moving = false,
            .max_hp = 10,
            .current_hp = 10,
            .move_range = 4,
            .weapon_range = 8,
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
            .v_x = 12.0f,
            .v_y = (float)(2 + i * 3),
            .is_moving = false,
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
    data->selected_unit_index = 0;
    data->showing_movement = false;
    data->showing_targets = false;
    data->combat_over = false;

    return data;
}

void combat_mode_destroy(CombatModeData *data) {
    if (!data) return;
    if (data->grid) free(data->grid);
    if (data->reachable_tiles) free(data->reachable_tiles);
    free(data);
}

void combat_mode_update(CombatModeData *data, PlayerCommand cmd, float delta_time) {
    if (data->combat_over) {
        if (cmd != CMD_NONE) {
            game_state_transition_to(data->game_state, GAME_MODE_DUNGEON_EXPLORATION);
        }
        return;
    }
    
    if (!data->player_turn) {
        return;
    }
    
    if (data->showing_targets) {
        switch (cmd) {
            case CMD_MOVE_LEFT:
            case CMD_MOVE_UP:
                data->selected_target_index--;
                if (data->selected_target_index < 0) {
                    data->selected_target_index = data->enemy_count - 1;
                }
                while (data->selected_target_index >= 0 && 
                       !data->enemy_units[data->selected_target_index].is_alive) {
                    data->selected_target_index--;
                    if (data->selected_target_index < 0) {
                        data->selected_target_index = data->enemy_count - 1;
                    }
                }
                break;
                
            case CMD_MOVE_RIGHT:
            case CMD_MOVE_DOWN:
                data->selected_target_index++;
                if (data->selected_target_index >= data->enemy_count) {
                    data->selected_target_index = 0;
                }
                while (data->selected_target_index < data->enemy_count && 
                       !data->enemy_units[data->selected_target_index].is_alive) {
                    data->selected_target_index++;
                    if (data->selected_target_index >= data->enemy_count) {
                        data->selected_target_index = 0;
                    }
                }
                break;
                
            case CMD_ACTION:
                if (data->selected_target_index >= 0 && 
                    data->selected_target_index < data->enemy_count) {
                    CombatUnit *shooter = &data->player_units[data->selected_unit_index];
                    CombatUnit *target = &data->enemy_units[data->selected_target_index];
                    
                    if (can_shoot_target(data, shooter, target)) {
                        int hit_chance = calculate_hit_chance(data, shooter, target);
                        int roll = rand() % 100;
                        
                        if (roll < hit_chance) {
                            target->current_hp -= 3;
                            if (target->current_hp <= 0) {
                                target->current_hp = 0;
                                target->is_alive = false;
                                
                                CombatCell *target_cell = get_cell(data, target->grid_x, target->grid_y);
                                if (target_cell) {
                                    target_cell->occupant = NULL;
                                }
                            }
                        }
                        
                        shooter->has_acted = true;
                        data->showing_targets = false;
                    }
                }
                break;
                
            case CMD_COMBAT_TEST:
                data->showing_targets = false;
                break;
                
            default:
                break;
        }
        
        return;
    }
    
    if (data->showing_movement) {
        switch (cmd) {
            case CMD_MOVE_UP:
                data->cursor_y--;
                if (data->cursor_y < 0) data->cursor_y = 0;
                break;
                
            case CMD_MOVE_DOWN:
                data->cursor_y++;
                if (data->cursor_y >= data->grid_height) {
                    data->cursor_y = data->grid_height - 1;
                }
                break;
                
            case CMD_MOVE_LEFT:
                data->cursor_x--;
                if (data->cursor_x < 0) data->cursor_x = 0;
                break;
                
            case CMD_MOVE_RIGHT:
                data->cursor_x++;
                if (data->cursor_x >= data->grid_width) {
                    data->cursor_x = data->grid_width - 1;
                }
                break;
                
            case CMD_ACTION:
                int cursor_index = data->cursor_y * data->grid_width + data->cursor_x;
                
                if (data->reachable_tiles && data->reachable_tiles[cursor_index]) {
                    CombatUnit *unit = &data->player_units[data->selected_unit_index];
                    
                    CombatCell *old_cell = get_cell(data, unit->grid_x, unit->grid_y);
                    if (old_cell) {
                        old_cell->occupant = NULL;
                    }
                    
                    unit->grid_x = data->cursor_x;
                    unit->grid_y = data->cursor_y;
                    unit->has_moved = true;
                    unit->is_moving = true;
                    
                    CombatCell *new_cell = get_cell(data, unit->grid_x, unit->grid_y);
                    if (new_cell) {
                        new_cell->occupant = unit;
                    }
                    
                    data->showing_movement = false;
                    
                    for (int i = 0; i < data->enemy_count; i++) {
                        if (data->enemy_units[i].is_alive && 
                            can_shoot_target(data, unit, &data->enemy_units[i])) {
                            data->showing_targets = true;
                            data->selected_target_index = i;
                            break;
                        }
                    }
                }
                break;
                
            case CMD_COMBAT_TEST:
                data->showing_movement = false;
                break;
                
            default:
                break;
        }
        
        return;
    }
    
    switch (cmd) {
        case CMD_MOVE_LEFT:
            data->selected_unit_index--;
            if (data->selected_unit_index < 0) {
                data->selected_unit_index = data->player_count - 1;
            }
            break;
            
        case CMD_MOVE_RIGHT:
            data->selected_unit_index++;
            if (data->selected_unit_index >= data->player_count) {
                data->selected_unit_index = 0;
            }
            break;
            
        case CMD_COMBAT_TEST:
            if (data->selected_unit_index >= 0 && data->selected_unit_index < data->player_count) {
                CombatUnit *selected = &data->player_units[data->selected_unit_index];
                
                if (selected->has_moved) {
                    break;
                }
                
                calculate_reachable_tiles(data, selected);
                
                data->cursor_x = selected->grid_x;
                data->cursor_y = selected->grid_y;
                
                data->showing_movement = true;
            }
            break;
            
        default:
            break;
    }
    
    float combat_speed = 0.3f;
    for (int i = 0; i < data->player_count; i++) {
        update_unit_visual_position(&data->player_units[i], delta_time * combat_speed);
    }
    for (int i = 0; i < data->enemy_count; i++) {
        update_unit_visual_position(&data->enemy_units[i], delta_time * combat_speed);
    }
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

    for (int i = 0; i < combat_arena_background_count_2; i++) {
        int point_x = screen_center_x + combat_arena_background_2[i].x + art_offset_x;
        int point_y = screen_center_y + combat_arena_background_2[i].y + art_offset_y;
        buffer_draw_char(fb, point_x, point_y, '.', COLOR_BRIGHT_WHITE);
    }

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

            if (data->showing_movement && data->reachable_tiles) {
                int tile_index = y * data->grid_width + x;
                if (data->reachable_tiles[tile_index]) {
                    if (!cell->occupant) {
                        ch = '*';
                        color = COLOR_GREEN;
                    }
                }
            }

            buffer_draw_char(fb, screen_x, screen_y, ch, color);
        }
    }

    for (int i = 0; i < data->player_count; i++) {
        CombatUnit *unit = &data->player_units[i];
        if (unit->is_alive) {
            int screen_x = grid_start_x + (int)unit->v_x;
            int screen_y = grid_start_y + (int)unit->v_y;
            buffer_draw_char(fb, screen_x, screen_y, unit->display_char, unit->color);
        }
    }

    for (int i = 0; i < data->enemy_count; i++) {
        CombatUnit *unit = &data->enemy_units[i];
        if (unit->is_alive) {
            int screen_x = grid_start_x + (int)unit->v_x;
            int screen_y = grid_start_y + (int)unit->v_y;
            buffer_draw_char(fb, screen_x, screen_y, unit->display_char, unit->color);
        }
    }

    if (data->showing_targets && data->selected_target_index >= 0 && 
            data->selected_target_index < data->enemy_count) {
        CombatUnit *shooter = &data->player_units[data->selected_unit_index];
        CombatUnit *target = &data->enemy_units[data->selected_target_index];

        if (target->is_alive) {
            int screen_x = grid_start_x + (int)target->v_x;
            int screen_y = grid_start_y + (int)target->v_y;

            buffer_draw_char(fb, screen_x - 1, screen_y, '{', COLOR_RED);
            buffer_draw_char(fb, screen_x + 1, screen_y, '}', COLOR_RED);

            int hit_chance = calculate_hit_chance(data, shooter, target);
            char chance_text[32];
            snprintf(chance_text, sizeof(chance_text), "%d%% hit", hit_chance);

            int text_x = (data->game_state->term_width - strlen(chance_text)) / 2;
            draw_text(fb, text_x, screen_y - 2, chance_text, COLOR_YELLOW);
        }
    }

    //cursor drawing
    if (data->showing_movement) {
        int screen_x = grid_start_x + data->cursor_x;
        int screen_y = grid_start_y + data->cursor_y;

        int cursor_index = data->cursor_y * data->grid_width + data->cursor_x;
        bool valid = data->reachable_tiles && data->reachable_tiles[cursor_index];

        Color cursor_color = valid ? COLOR_CYAN : COLOR_RED;
        buffer_draw_char(fb, screen_x - 1, screen_y, '<', cursor_color);
        buffer_draw_char(fb, screen_x + 1, screen_y, '>', cursor_color);
    }

    if (data->selected_unit_index >= 0 && data->selected_unit_index < data->player_count && data->player_turn) {
        CombatUnit *selected = &data->player_units[data->selected_unit_index];
        int screen_x = grid_start_x + (int)selected->v_x;
        int screen_y = grid_start_y + (int)selected->v_y;

        buffer_draw_char(fb, screen_x - 1, screen_y, '[', COLOR_CYAN);
        buffer_draw_char(fb, screen_x + 1, screen_y, ']', COLOR_CYAN);
        buffer_draw_char(fb, screen_x, screen_y, selected->display_char, COLOR_CYAN);
    }

    int ui_top_y = frame_y - 2;
    draw_text_centered(fb, ui_top_y, 
            data->player_turn ? "=== PLAYER TURN ===" : "=== ENEMY TURN ===", 
            COLOR_YELLOW);

    int ui_bottom_y = frame_y + frame_height + 1;
    if (data->showing_targets) {
        draw_text_centered(fb, ui_bottom_y,
                "| Arrows: select target | Space: shoot | C: cancel |", 
                COLOR_RED);
    } else if (data->showing_movement) {
        draw_text_centered(fb, ui_bottom_y,
                "| Arrows: move cursor | Space: confirm | C: cancel |", 
                COLOR_CYAN);
    } else {
        draw_text_centered(fb, ui_bottom_y,
                "| Left/Right: select unit | C: move | Q: exit |", 
                COLOR_CYAN);
    }
    draw_text_centered(fb, ui_bottom_y + 1,
                      "-----------------------------------------------", 
                      COLOR_BRIGHT_BLACK);

    draw_text_centered(fb, ui_bottom_y - 1,
                      "-----------------------------------------------", 
                      COLOR_BRIGHT_BLACK);

    if (data->selected_unit_index >= 0 && data->selected_unit_index < data->player_count) {
        CombatUnit *unit = &data->player_units[data->selected_unit_index];
        char info[64];
        snprintf(info, sizeof(info), "HP: %d/%d | Move: %d | Range: %d", 
                unit->current_hp, unit->max_hp, unit->move_range, unit->weapon_range);
        draw_text_centered(fb, ui_bottom_y + 1, info, COLOR_CYAN);
    }
}
