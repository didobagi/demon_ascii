#ifndef INPUT_H
#define INPUT_H

typedef enum {
    KEY_NONE = 0,
    KEY_UP = 1,
    KEY_DOWN = 2,
    KEY_LEFT = 3,
    KEY_RIGHT = 4,
    KEY_MORPH = 5,
    KEY_QUIT = 'q'
} GameKey;

void enable_raw_mode(void);
void disable_raw_mode(void);
GameKey read_game_key(void);

#endif
