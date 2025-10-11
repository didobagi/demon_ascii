#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>

#include "../include/types.h"
#include "../include/shapes.h"
#include "../include/render.h"
#include "../include/game.h"
#include "../include/input.h"
#include "../include/splash.h"
#include "../include/morph.h"

void init (int *width, int *height) {
    usleep(50000);
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *height = w.ws_row;
    *width = w.ws_col;
    printf("\033[?1049h");
    printf("\033[2J");
    printf("\033[?25l"); 
}

void initialise_game (GameState *game) {
    init(&game->max_x, &game->max_y);
    enable_raw_mode();
    atexit(disable_raw_mode);
    for (int row = 0;row < SECTOR_ROWS; row++) {
        for (int col= 0;col < SECTOR_COLS;col++) {
            int random_val = rand() % 100;
            if (random_val < 0) {
                game->sectors[row][col].is_dangerous = true;
            } else {
                game->sectors[row][col].is_dangerous = false;
            }
            game->sectors[row][col].state_change_frame = 0;
        }
    }
    game->object_count = 0;

    create_object(game->objects, &game->object_count,
            demon_template, demon_point_count,
            12.0, 12.0, 1.0, 0.0,
            6.0, TEXTURE_GRADIENT,
            COLOR_BRIGHT_RED);

    setup_morph_forms(&game->objects[0],
                      demon_template, demon_point_count, 
                      demon_snake_template, 50);

    game->frame = 0;
}

static void update_sectors (GameState *game) {
    if (game->frame % 60 != 0) return;

    int rand_row = rand() % SECTOR_ROWS;
    int rand_col = rand() % SECTOR_COLS;
    
    game->sectors[rand_row][rand_col].is_dangerous =
        !game->sectors[rand_row][rand_col].is_dangerous;
    game->sectors[rand_row][rand_col].state_change_frame = game->frame;
}

static void handle_input(GameState *game) {
    GameKey key = read_game_key();
    if (key == KEY_NONE) return;
    
    if (key == KEY_QUIT) {
        disable_raw_mode();
        printf("\033[2J\033[1;1H");
        exit(0);
    }
    
    if (key == KEY_MORPH) {
        initiate_morph(&game->objects[0]);
        return;
    }
    float step_size = 2.0;
    float new_x = game->objects[0].transform.x;
    float new_y = game->objects[0].transform.y;
    
    switch (key) {
        case KEY_UP:    new_y -= step_size; break;
        case KEY_DOWN:  new_y += step_size; break;
        case KEY_LEFT:  new_x -= step_size; break;
        case KEY_RIGHT: new_x += step_size; break;
        default: return;
    }
    
    bool in_bounds = (new_x + game->objects[0].bounds.min_x > 0 && 
                     new_x + game->objects[0].bounds.max_x < game->max_x &&
                     new_y + game->objects[0].bounds.min_y >= 1 && 
                     new_y + game->objects[0].bounds.max_y <= game->max_y);
    
    if (in_bounds) {
        game->objects[0].transform.x = new_x;
        game->objects[0].transform.y = new_y;

        if (game->objects[0].in_snake_form) {
            game->objects[0].bounds = calculate_shape_bounds_selective(
                game->objects[0].shape.original_points,
                game->objects[0].shape.point_count,
                game->objects[0].point_collected,
                game->objects[0].in_snake_form
                );
        }
        update_visual(&game->objects[0]);
    }
}

void render_game(GameState *game) {
    render_background(game->max_x, game->max_y, game->frame, game->sectors);
    render_collectibles(game->collectibles, game->collectible_count, game->frame);

    for (int i = 0;i < game->object_count;i ++) {
        if(game->objects[i].active) {
            render(&game->objects[i], game->objects[i].transform.x,
                    game->objects[i].transform.y,false, game->frame);
        }
    }
    if (game->objects[0].in_snake_form) {
        printf("\033[1;1H");
        printf("Points: %d, %d",
                game->objects[0].total_collected_count,
                game->objects[0].snake_form_point_count);
        fflush(stdout);
    }
}

static void show_splash_scr (int term_w, int term_h) {
    printf("\033[2J");
    fflush(stdout);

    const char *message = "PRESS ANY KEY TO START";
    int message_len = 22;

    int center_x = term_w / 2;
    int center_y = term_h / 2;
    int text_start_x = center_x - (message_len / 2);
    int text_y = center_y - 2;
    printf("\033[%d;%dH", text_y, text_start_x);
    printf("%s", message);
    fflush(stdout);
    
        struct termios original, raw;
    tcgetattr(STDIN_FILENO, &original);
    raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);  
    raw.c_cc[VMIN] = 1;   
    raw.c_cc[VTIME] = 0; 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    char c;

    read(STDIN_FILENO, &c, 1);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);

}
int main () {
    srand(time(NULL));

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col;
    int term_height = w.ws_row;

    show_splash_screen(term_width, term_height);

    GameState game;
    initialise_game(&game);

    while (1) {
        game.frame++;
        //update_sectors(&game);
        handle_input(&game);
        update_morph(&game.objects[0],&game, game.frame);
        if (game.objects[0].in_snake_form) {
            check_collectible_collision(&game.objects[0], &game);
        }
        bounce(&game.objects[0], game.max_x, game.max_y); 
        render_game(&game);
        //update_transform(&objects[0], max_x, max_y);
        //bool hit_b = check_boundaries(&objects[0], max_x, max_y);
        //commit_transf(&objects[0]);
        //update_visual(&objects[0]);
        usleep(33000);
    }
    return 0;
}
