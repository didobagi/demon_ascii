#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "../include/types.h"
#include "../include/frame_buffer.h"
#include "../include/world.h"
#include "../include/input.h"
#include "../include/controls.h"
#include "../include/camera.h"
#include "../include/render.h"
#include "../include/map_template.h"
#include "../include/map_builder.h"
#include "../include/shapes.h"
#include "../include/movement.h"

#define FRAME_TIME 0.016f

FILE *debug_log = NULL;
static bool moved_this_frame = false;

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


void handle_player_command(World *world, GameObject *player, PlayerCommand cmd) {
    extern bool moved_this_frame;     
    if (cmd == CMD_NONE || moved_this_frame) return;
    
    int dx = 0, dy = 0;
    
    switch (cmd) {
        case CMD_MOVE_UP:    dy = -1; break;
        case CMD_MOVE_DOWN:  dy = 1; break;
        case CMD_MOVE_LEFT:  dx = -1; break;
        case CMD_MOVE_RIGHT: dx = 1; break;
        default: return;
    }
    
    if (movement_try_mov(player, world, dx, dy)) {
        moved_this_frame = true;
    }
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

    const int WORLD_WIDTH = 400;
    const int WORLD_HEIGHT = 400;

    World *world = create_world(WORLD_WIDTH, WORLD_HEIGHT);
    if (!world) {
        fprintf(stderr, "Failed to create world\n");
        if (debug_log) fclose(debug_log);
        return 1;
    }

    TemplateLibrary *library = create_template_library();
    Template *test_room = load_template_from_file("templates/test_room.txt");
    if (test_room) {
        add_template_to_library(library, test_room);
    }

    MapGenParams params = {
        .min_rooms = 5,
        .max_rooms = 10,
        .room_spacing = 10,
        .max_placement_attempts = 100,
        .extra_corridor_chance = 0.0f
    };

    MapGenResult gen_result = generate_dungeon(world, library, params);

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

    player.shape.original_points = demon_template;
    player.shape.point_count = demon_point_count;
    player.shape.texture = TEXTURE_FIRE;

    static Point player_rotated[100];
    player.shape.rotated_points = player_rotated;

    static float player_distances[100];
    player.shape.distances = player_distances;

    for (int i = 0; i < player.shape.point_count; i++) {
        float x = demon_template[i].x;
        float y = demon_template[i].y;
        player.shape.distances[i] = sqrt(x*x + y*y);
        player.shape.rotated_points[i] = demon_template[i];
    }

    world_add_entity(world, player.cell_x, player.cell_y, &player);
    movement_init_entity(&player);

    Camera camera;
    camera_init(&camera, term_width, term_height);

    FrameBuffer fb;
    unsigned int frame = 0;

    while (1) {
        frame++;
        moved_this_frame = false;

        PlayerCommand cmd = read_player_command();
        if (cmd == CMD_QUIT) {
            break;
        }

        handle_player_command(world, &player, cmd);
        movement_update(&player, FRAME_TIME);
        camera_follow_entity_smooth(&camera, &player, WORLD_WIDTH, WORLD_HEIGHT);
        render_world(&fb, world, &camera, frame);

        usleep(16000);
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
