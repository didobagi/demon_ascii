#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>

#include "../include/types.h"
#include "../include/input.h"
#include "../include/controls.h"
#include "../include/game_state.h"

#define FRAME_TIME 0.016f

FILE *debug_log = NULL;

void init_term(int *width, int *height) {
    usleep(50000);
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *height = w.ws_row;
    *width = w.ws_col;
    printf("\033[?1049h");  
    printf("\033[2J"); 
    printf("\033[?25l");
}

int main() {
    srand(time(NULL));

    debug_log = fopen("generation.log", "w");
    if (debug_log) {
        fprintf(debug_log, "=== Game Starting ===\n\n");
        fflush(debug_log);
    }

    int term_width, term_height;
    init_term(&term_width, &term_height);
    enable_raw_mode();
    atexit(disable_raw_mode);

    GameState *game = game_state_create(term_width, term_height);
    if (!game) {
        fprintf(stderr, "Failed to create game state\n");
        if (debug_log) fclose(debug_log);
        return 1;
    }

    while (game->current_mode != GAME_MODE_QUIT) {
        PlayerCommand cmd = read_player_command();
        if (cmd == CMD_QUIT) {
            game_state_transition_to(game, GAME_MODE_QUIT);
        }
        
        game_state_update(game, cmd, FRAME_TIME);
        
        game_state_update_transition(game);
        
        FrameBuffer fb;
        game_state_render(game, &fb);
        
        usleep(16000);
    }

    if (debug_log) {
        fprintf(debug_log, "\n=== Game Ending ===\n");
        fclose(debug_log);
    }

    game_state_destroy(game);
    disable_raw_mode();
    printf("\033[2J\033[1;1H");

    return 0;
}
