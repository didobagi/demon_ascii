#ifndef MODE_DUNGEON_H
#define MODE_DUNGEON_H

#include "types.h"
#include "world.h"
#include "camera.h"
#include "frame_buffer.h"
#include "map_builder.h"
#include "map_template.h"
#include "input.h"
#include "game_state.h"

typedef enum {
    DUNGEON_STATE_PLAYING,
    DUNGEON_STATE_TRANSITIONING,
} DungeonState;

typedef struct DungeonModeData {
    World *world;
    MapGenResult gen_result;
    TemplateLibrary *template_library;
    
    GameObject player;
    
    GameObject *all_enemies[100];
    int enemy_count;
    
    Camera camera;
    
    unsigned int frame;
    
    bool moved_this_frame;
    
    GameState *game_state;
    
    DungeonState state;
    float transition_timer;
    GameMode transition_target;
    GameObject *transition_enemy;
} DungeonModeData;

DungeonModeData* dungeon_mode_create(GameState *game_state);
void dungeon_mode_destroy(DungeonModeData *data);

void dungeon_mode_update(DungeonModeData *data, PlayerCommand cmd, float delta_time);
void dungeon_mode_render(DungeonModeData *data, FrameBuffer *fb);

void dungeon_handle_player_command(DungeonModeData *data, PlayerCommand cmd);

#endif
