#include "../include/input.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static struct termios original_termios;

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &original_termios);
    
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    printf("\033[?25h");
    printf("\033[?1049l");
    fflush(stdout);
}

GameKey read_game_key(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) {
        return KEY_NONE;
    }
    
    if (c == 27) {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_NONE;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_NONE;
        
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
            }
        }
        return KEY_NONE;
    }
    
    if (c == 'q' || c == 'Q') return KEY_QUIT;
    if (c == 'w' || c == 'W') return KEY_UP;
    if (c == 's' || c == 'S') return KEY_DOWN;
    if (c == 'a' || c == 'A') return KEY_LEFT;
    if (c == 'd' || c == 'D') return KEY_RIGHT;
    if (c == 'c' || c == 'C') return KEY_COMBAT_TEST;
    if (c == ' ') return KEY_ACTION;  
    if (c == 'm' || c == 'M') return KEY_MORPH;  
    
    return KEY_NONE;
}

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
            
        case KEY_ACTION:
            return CMD_ACTION;
            
        case KEY_MORPH:
            return CMD_MORPH;
            
        case KEY_COMBAT_TEST:
            return CMD_COMBAT_TEST;
            
        case KEY_QUIT:
            return CMD_QUIT;
            
        default:
            return CMD_NONE;
    }
}
