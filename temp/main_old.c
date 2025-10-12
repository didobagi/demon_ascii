#include <asm-generic/ioctls.h>
#include <bits/posix2_lim.h>
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
#include "../include/world.h"

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
            if (random_val < 30) {
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
            12.0, 12.0, 0.4, 0.2,
            6.0, TEXTURE_GRADIENT);

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
        update_visual(&game->objects[0]);
    }
}

void render_game(GameState *game) {
    render_background(game->max_x, game->max_y, game->frame, game->sectors);

    for (int i = 0;i < game->object_count;i ++) {
        if(game->objects[i].active) {
            render(&game->objects[i], game->objects[i].transform.x,
                    game->objects[i].transform.y,false, game->frame);
        }
    }
}

static void show_splash_scr (int term_w, int term_h) {
    printf("\033[2J");
    printf("\033[?25l");
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

int main() {
    // === INITIALIZATION ===
    
    // Set up terminal for the game
    int term_width, term_height;
    init(&term_width, &term_height);
    enable_raw_mode();
    atexit(disable_raw_mode);
    
    // Create the world - 200x200 cells for testing
    World *world = create_world(200, 200);
    if (!world) {
        printf("Failed to create world\n");
        return 1;
    }
    
    // Design a simple test map
    // Walls around perimeter
    for (int x = 0; x < 200; x++) {
        world_set_terrain(world, x, 0, TERRAIN_WALL);
        world_set_terrain(world, x, 199, TERRAIN_WALL);
    }
    for (int y = 0; y < 200; y++) {
        world_set_terrain(world, 0, y, TERRAIN_WALL);
        world_set_terrain(world, 199, y, TERRAIN_WALL);
    }
    
    // Add a few interior walls for testing collision
    for (int x = 50; x < 70; x++) {
        world_set_terrain(world, x, 50, TERRAIN_WALL);
    }
    
    // Create player in center of world
    GameObject player = {0};  // Zero initialize everything
    player.entity_type = ENTITY_PLAYER;
    player.active = true;
    player.cell_x = 100;
    player.cell_y = 100;
    player.v_x = 100.0f;
    player.v_y = 100.0f;
    player.color = COLOR_BRIGHT_RED;
    
    // Give player a simple shape (just a single character for now)
    static Point player_point = {0, 0};
    player.shape.original_points = &player_point;
    player.shape.point_count = 1;
    
    // Register player with world
    world_add_entity(world, player.cell_x, player.cell_y, &player);
    
    // Create camera centered on player
    Camera camera;
    camera.width = term_width;
    camera.height = term_height;
    camera.x = player.cell_x - camera.width / 2;
    camera.y = player.cell_y - camera.height / 2;
    
    // Clamp camera to world bounds
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x + camera.width > 200) camera.x = 200 - camera.width;
    if (camera.y + camera.height > 200) camera.y = 200 - camera.height;
    
    // Create frame buffer
    FrameBuffer fb;
    
    // === GAME LOOP ===
    
    while (1) {
        // --- INPUT ---
        GameKey key = read_game_key();
        
        if (key == KEY_QUIT) {
            break;
        }
        
        if (key != KEY_NONE) {
            // Calculate target cell based on input
            int target_x = player.cell_x;
            int target_y = player.cell_y;
            
            switch (key) {
                case KEY_UP:    target_y--; break;
                case KEY_DOWN:  target_y++; break;
                case KEY_LEFT:  target_x--; break;
                case KEY_RIGHT: target_x++; break;
                default: break;
            }
            
            // Check if target cell is walkable
            if (world_is_walkable(world, target_x, target_y)) {
                // Remove player from old cell
                world_remove_entity(world, player.cell_x, player.cell_y, &player);
                
                // Update player position
                player.cell_x = target_x;
                player.cell_y = target_y;
                player.v_x = (float)target_x;
                player.v_y = (float)target_y;
                
                // Add player to new cell
                world_add_entity(world, player.cell_x, player.cell_y, &player);
            }
        }
        
        // --- UPDATE ---
        
        // Update camera to follow player
        camera.x = player.cell_x - camera.width / 2;
        camera.y = player.cell_y - camera.height / 2;
        
        // Clamp camera to world bounds
        if (camera.x < 0) camera.x = 0;
        if (camera.y < 0) camera.y = 0;
        if (camera.x + camera.width > 200) camera.x = 200 - camera.width;
        if (camera.y + camera.height > 200) camera.y = 200 - camera.height;
        
        // --- RENDER ---
        
        init_frame_buffer(&fb, term_width, term_height);
        
        // Render terrain
        for (int screen_y = 0; screen_y < camera.height; screen_y++) {
            for (int screen_x = 0; screen_x < camera.width; screen_x++) {
                int world_x = camera.x + screen_x;
                int world_y = camera.y + screen_y;
                
                TerrainType terrain = world_get_terrain(world, world_x, world_y);
                char ch = (terrain == TERRAIN_WALL) ? '#' : '.';
                Color color = (terrain == TERRAIN_WALL) ? COLOR_WHITE : COLOR_BRIGHT_BLACK;
                
                buffer_draw_char(&fb, screen_x + 1, screen_y + 1, ch, color);
            }
        }
        
        // Render player
        int player_screen_x = (int)player.v_x - camera.x + 1;
        int player_screen_y = (int)player.v_y - camera.y + 1;
        buffer_draw_char(&fb, player_screen_x, player_screen_y, '@', player.color);
        
        present_frame(&fb);
        
        usleep(33000);  // ~30 FPS
    }
    
    // === CLEANUP ===
    
    destroy_world(world);
    disable_raw_mode();
    printf("\033[2J\033[1;1H");  // Clear screen and move cursor to top
    
    return 0;
}
