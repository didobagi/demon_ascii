#ifndef MODE_DIALOGUE_H
#define MODE_DIALOGUE_H

#include "controls.h"
#include "frame_buffer.h"
#include "dialogue.h"

struct GameState;
struct GameObject;

typedef struct DialogueModeData{
    GameState *game_state;

    GameObject *enemy;
    GameObject *player;

    DialogueFragment *current_fragment;
    DialogueFragment *fragment_pool;
    int fragment_pool_size;

    int selected_choice;
} DialogueModeData;

DialogueModeData* dialogue_mode_create (GameState *game_state, GameObject *enemy, GameObject *player);
void dialogue_mode_destroy (DialogueModeData *data);
void dialogue_mode_update (DialogueModeData *data, PlayerCommand cdm);
void dialogue_mode_render (DialogueModeData *data, FrameBuffer *fb);

#endif
