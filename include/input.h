#ifndef INPUT_H
#define INPUT_H

typedef enum {
    CMD_NONE,
    CMD_MOVE_UP,
    CMD_MOVE_DOWN,
    CMD_MOVE_LEFT,
    CMD_MOVE_RIGHT,
    CMD_ACTION,
    CMD_MORPH,
    CMD_COMBAT_TEST,
    CMD_QUIT,
} PlayerCommand;

typedef enum {
    KEY_NONE = 0,
    KEY_UP = 1,
    KEY_DOWN = 2,
    KEY_LEFT = 3,
    KEY_RIGHT = 4,
    KEY_ACTION = ' ',
    KEY_MORPH = 5,
    KEY_COMBAT_TEST = 6,
    KEY_QUIT = 'q'
} GameKey;

PlayerCommand read_player_command(void);
void enable_raw_mode(void);
void disable_raw_mode(void);
GameKey read_game_key(void);

#endif
