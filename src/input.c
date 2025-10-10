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
    
    return KEY_NONE;
}

