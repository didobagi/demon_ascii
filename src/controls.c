#include "../include/controls.h"
#include "../include/input.h"

PlayerCommand read_player_command(void) {
    GameKey key = read_game_key();
    
    switch (key) {
        case KEY_UP:
            return CMD_MOVE_UP;
            
        case KEY_DOWN:
            return CMD_MOVE_DOWN;
            
        case KEY_LEFT:
            return CMD_MOVE_LEFT;
            
        case KEY_RIGHT:
            return CMD_MOVE_RIGHT;
            
        case KEY_MORPH:
            return CMD_MORPH;
            
        case KEY_QUIT:
            return CMD_QUIT;
            
        default:
            return CMD_NONE;
    }
}
