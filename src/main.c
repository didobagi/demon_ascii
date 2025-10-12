#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>

#include "../include/types.h"
#include "../include/frame_buffer.h"
#include "../include/world.h"
#include "../include/input.h"
#include "../include/map_template.h"
#include "../include/map_builder.h"

FILE *debug_log = NULL;

void init_term(int *width, int *height) {
    usleep(50000);
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *height = w.ws_row;
    *width = w.ws_col;
    printf("\033[?1049h");  // Switch to alternate screen buffer
    printf("\033[2J");       // Clear screen
    printf("\033[?25l");     // Hide cursor
}

// Update camera position to follow the player
void update_camera(Camera *camera, GameObject *player, int world_width, int world_height) {
    // Center camera on player
    camera->x = player->cell_x - camera->width / 2;
    camera->y = player->cell_y - camera->height / 2;
    
    // Clamp camera to world boundaries
    if (camera->x < 0) camera->x = 0;
    if (camera->y < 0) camera->y = 0;
    if (camera->x + camera->width > world_width) {
        camera->x = world_width - camera->width;
    }
    if (camera->y + camera->height > world_height) {
        camera->y = world_height - camera->height;
    }
}

// Render the visible portion of the world
void render_world(FrameBuffer *fb, World *world, Camera *camera, GameObject *player) {
    // Clear the frame buffer
    init_frame_buffer(fb, camera->width, camera->height);
    
    // Render terrain for all visible cells
    for (int screen_y = 0; screen_y < camera->height; screen_y++) {
        for (int screen_x = 0; screen_x < camera->width; screen_x++) {
            // Convert screen coordinates to world coordinates
            int world_x = camera->x + screen_x;
            int world_y = camera->y + screen_y;
            
            // Get terrain type for this cell
            TerrainType terrain = world_get_terrain(world, world_x, world_y);
            
            // Choose character and color based on terrain
            char ch;
            Color color;
            if (terrain == TERRAIN_WALL) {
                ch = '#';
                color = COLOR_WHITE;
            } else {
                ch = '.';
                color = COLOR_BRIGHT_BLACK;
            }
            
            // Draw to frame buffer (add 1 for terminal's 1-based coordinates)
            buffer_draw_char(fb, screen_x + 1, screen_y + 1, ch, color);
        }
    }
    
    // Render the player
    // Convert player's world position to screen position
    int player_screen_x = (int)player->v_x - camera->x + 1;
    int player_screen_y = (int)player->v_y - camera->y + 1;
    
    // Only draw player if they're within the visible viewport
    if (player_screen_x >= 1 && player_screen_x <= camera->width &&
        player_screen_y >= 1 && player_screen_y <= camera->height) {
        buffer_draw_char(fb, player_screen_x, player_screen_y, '@', player->color);
    }
    
    // Display the completed frame
    present_frame(fb);
}

void handle_player_movement(World *world, GameObject *player, GameKey key) {
    if (key == KEY_NONE) return;
    
    int target_x = player->cell_x;
    int target_y = player->cell_y;
    
    switch (key) {
        case KEY_UP:    target_y--; break;
        case KEY_DOWN:  target_y++; break;
        case KEY_LEFT:  target_x--; break;
        case KEY_RIGHT: target_x++; break;
        default: return;
    }
    
    if (world_is_walkable(world, target_x, target_y)) {
        world_remove_entity(world, player->cell_x, player->cell_y, player);
        
        player->cell_x = target_x;
        player->cell_y = target_y;
        
        player->v_x = (float)target_x;
        player->v_y = (float)target_y;
        
        world_add_entity(world, player->cell_x, player->cell_y, player);
    }
}

// Create and initialize a simple test map
void create_test_map(World *world) {
    int width = 200;
    int height = 200;
    
    for (int x = 0; x < width; x++) {
        world_set_terrain(world, x, 0, TERRAIN_WALL);           
        world_set_terrain(world, x, height - 1, TERRAIN_WALL);  
    }
    for (int y = 0; y < height; y++) {
        world_set_terrain(world, 0, y, TERRAIN_WALL);           
        world_set_terrain(world, width - 1, y, TERRAIN_WALL);   
    }
    
    for (int x = 50; x < 70; x++) {
        world_set_terrain(world, x, 50, TERRAIN_WALL);
    }
    for (int y = 80; y < 100; y++) {
        world_set_terrain(world, 120, y, TERRAIN_WALL);
    }
}

int main() {
    srand(time(NULL));
    
    debug_log = fopen("generation.log", "w");
    if (debug_log) {
        fprintf(debug_log, "=== Dungeon Generation Log ===\n\n");
        fflush(debug_log);
    }
    
    
    int term_width, term_height;
    init_term(&term_width, &term_height);
    enable_raw_mode();
    atexit(disable_raw_mode);
    
    const int WORLD_WIDTH = 200;
    const int WORLD_HEIGHT = 200;
    
    World *world = create_world(WORLD_WIDTH, WORLD_HEIGHT);
    if (!world) {
        fprintf(stderr, "Failed to create world\n");
        if (debug_log) fclose(debug_log);
        return 1;
    }
    
    if (debug_log) {
        fprintf(debug_log, "World created: %dx%d cells\n\n", WORLD_WIDTH, WORLD_HEIGHT);
        fflush(debug_log);
    }
    
    TemplateLibrary *library = create_template_library();
    if (!library) {
        fprintf(stderr, "Failed to create template library\n");
        destroy_world(world);
        if (debug_log) fclose(debug_log);
        return 1;
    }
    
    Template *test_room = load_template_from_file("templates/test_room.txt");
    if (test_room) {
        if (debug_log) {
            fprintf(debug_log, "Loaded template: %s (%dx%d)\n", 
                    test_room->name, test_room->width, test_room->height);
            fflush(debug_log);
        }
        add_template_to_library(library, test_room);
    } else {
        fprintf(stderr, "Warning: Could not load test_room.txt\n");
    }

    if (debug_log) {
        fprintf(debug_log, "Template library contains %d templates\n\n", library->count);
        fflush(debug_log);
    }
    
    MapGenParams params;
    params.min_rooms = 5;           
    params.max_rooms = 10;         
    params.room_spacing = 10;     
    params.max_placement_attempts = 100;  
    params.extra_corridor_chance = 0.0f; 
    
    if (debug_log) {
        fprintf(debug_log, "Generation parameters:\n");
        fprintf(debug_log, "  Target rooms: %d-%d\n", params.min_rooms, params.max_rooms);
        fprintf(debug_log, "  Room spacing: %d cells\n", params.room_spacing);
        fprintf(debug_log, "  Max placement attempts: %d\n\n", params.max_placement_attempts);
        fflush(debug_log);
    }
    
    MapGenResult gen_result = generate_dungeon(world, library, params);
    
    if (debug_log) {
        fprintf(debug_log, "\n=== Generation Complete ===\n");
        fprintf(debug_log, "Success: %s\n", gen_result.success ? "YES" : "NO");
        fprintf(debug_log, "Rooms placed: %d\n", gen_result.room_count);
        fprintf(debug_log, "\nRoom locations:\n");
        for (int i = 0; i < gen_result.room_count; i++) {
            PlacedRoom *room = &gen_result.rooms[i];
            fprintf(debug_log, "  %d. %s at (%d, %d) size %dx%d center (%d, %d)\n",
                    i + 1, room->name, room->x, room->y, 
                    room->width, room->height,
                    room->center_x, room->center_y);
        }
        fprintf(debug_log, "\n");
        fflush(debug_log);
    }
    
    
    GameObject player;
    memset(&player, 0, sizeof(GameObject));
    player.entity_type = ENTITY_PLAYER;
    player.active = true;
    
    if (gen_result.room_count > 0) {
        player.cell_x = gen_result.rooms[0].center_x;
        player.cell_y = gen_result.rooms[0].center_y;
    } else {
        player.cell_x = WORLD_WIDTH / 2;
        player.cell_y = WORLD_HEIGHT / 2;
    }
    
    player.v_x = (float)player.cell_x;
    player.v_y = (float)player.cell_y;
    player.color = COLOR_BRIGHT_RED;
    
    world_add_entity(world, player.cell_x, player.cell_y, &player);
    
    if (debug_log) {
        fprintf(debug_log, "Player spawned at (%d, %d)\n\n", player.cell_x, player.cell_y);
        fprintf(debug_log, "=== Game Starting ===\n");
        fflush(debug_log);
    }
    
    Camera camera;
    camera.width = term_width;
    camera.height = term_height;
    update_camera(&camera, &player, WORLD_WIDTH, WORLD_HEIGHT);
    
    FrameBuffer fb;
    
    
    while (1) {
        GameKey key = read_game_key();
        
        if (key == KEY_QUIT) {
            break;
        }
        
        handle_player_movement(world, &player, key);
        update_camera(&camera, &player, WORLD_WIDTH, WORLD_HEIGHT);
        render_world(&fb, world, &camera, &player);
        
        usleep(33000);
    }
    
    
    if (debug_log) {
        fprintf(debug_log, "\n=== Game Ending ===\n");
        fclose(debug_log);
    }
    
    free_map_gen_result(&gen_result);
    destroy_template_library(library);
    destroy_world(world);
    disable_raw_mode();
    printf("\033[2J\033[1;1H");
    
    return 0;
}
