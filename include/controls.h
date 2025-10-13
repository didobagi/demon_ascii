#ifndef CONTROLS_H
#define CONTROLS_H

typedef enum {
    CMD_NONE,
    CMD_MOVE_UP,
    CMD_MOVE_DOWN,
    CMD_MOVE_LEFT,
    CMD_MOVE_RIGHT,
    CMD_MORPH,
    CMD_QUIT,
} PlayerCommand;

PlayerCommand read_player_command(void);

#endif
