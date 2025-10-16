#include "../include/frame_buffer.h"
#include "../include/render.h"
#include "../include/world.h"
#include "../include/frame_buffer.h"
#include "../include/camera.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

void init_frame_buffer(FrameBuffer *fb, int width, int height) {
    fb->width = width;
    fb->height = height;

    for (int y = 0;y < height;y ++) {
        for (int x = 0;x < width;x ++) {
            fb->cells[y][x] = ' ';
            fb->colors[y][x] = COLOR_BLACK;
        }
    }
}

void buffer_draw_char(FrameBuffer *fb, int x, int y, char c, Color color) {
    if (x >= 1 && x <= fb->width && y >= 1 &&  y <= fb->height) {
        fb->cells[y-1][x-1] = c;
        fb->colors[y-1][x-1] = color;
    }
}

void present_frame(FrameBuffer *fb) {
    static char output_buffer[400000];
    int pos = 0;
    Color current_color = -1;

    static const char* COLOR_CODES[] = {
        "\033[30m", "\033[31m", "\033[32m", "\033[33m",
        "\033[34m", "\033[35m", "\033[36m", "\033[37m",
        "\033[90m", "\033[91m", "\033[92m", "\033[93m",
        "\033[94m", "\033[95m", "\033[96m", "\033[97m"
    };

    for (int y = 0; y < fb->height; y++) {
        int run_start = -1;
        
        for (int x = 0; x <= fb->width; x++) {              bool changed = (x < fb->width) && 
                          (fb->cells[y][x] != fb->prev_cells[y][x] || 
                           fb->colors[y][x] != fb->prev_colors[y][x]);
            
            if (changed) {
                if (run_start == -1) {
                    run_start = x;
                    pos += sprintf(output_buffer + pos, "\033[%d;%dH", y + 1, x + 1);
                }
                
                Color cell_color = fb->colors[y][x];
                if (cell_color != current_color) {
                    pos += sprintf(output_buffer + pos, "%s", COLOR_CODES[cell_color]);
                    current_color = cell_color;
                }
                output_buffer[pos++] = fb->cells[y][x];
                
                fb->prev_cells[y][x] = fb->cells[y][x];
                fb->prev_colors[y][x] = fb->colors[y][x];
                
            } else if (run_start != -1) {
                run_start = -1;
            }
        }
    }
    
    if (pos > 0) {
        pos += sprintf(output_buffer + pos, "\033[0m");
        write(STDOUT_FILENO, output_buffer, pos);
    }
}

char get_char_for_distance (float distance) {
    if (distance < 2.0) return '*';
    if (distance < 4.0) return '+';
    if (distance < 6.0) return '=';
    return '-';
}

void get_check (int world_x, int world_y, int sqr_size, char *ch, Color *color) {
    float ang = 44.0f * M_PI / 180.0f;
    int rot_x = (int)(world_x * cos(ang) - world_y * sin(ang));
    int rot_y = (int)(world_x * sin(ang) + world_y * cos(ang));
    if ((rot_x / sqr_size - (rot_y) / sqr_size) % 2 == 0) {
        *ch = 'W';
        *color = COLOR_BRIGHT_BLACK;
    } else {
        *ch = '.';
        *color = COLOR_BRIGHT_BLACK;
    }
}

char get_fire_char (int  point_index, float distance, unsigned int frame, float point_y) {
    float solid_tresh = 4.0;
    float fire_tresh = -2.0;
    
    if(point_y > solid_tresh) {
        return '#';
    }
    if (point_y < fire_tresh) {
    int phase = (point_index * 7 + (int)(distance * 3)) % 20;
    int cycle = (frame + phase) % 40;

    if (cycle < 10) return ' ';
    if (cycle < 20) return '@';
    if (cycle < 30) return '*';
    if (cycle < 35) return '+';
    return '.';
    }
    float transtion = (point_y - fire_tresh)/(solid_tresh - fire_tresh);
    int phase = (point_index * 7 + (int)(distance * 3)) % 20;
    int cycle_length = 40 + (int)(transtion*60);
    int cycle = (frame + phase) % cycle_length;

    int brigth_du = (int)(10 * (1.0 - transtion));
    if (cycle < brigth_du) return '#';
    if (cycle < brigth_du * 2) return '@';
    return '#';
}

void render_entity (FrameBuffer *fb, Camera *camera, GameObject *entity, unsigned int frame) {
    if (!entity->active) {
        return;
    }
    for (int i = 0;i < entity->shape.point_count;i ++) {
        if (entity->in_snake_form && !entity->point_collected[i]) {
            continue;
        }

        int point_world_x = (int)(entity->v_x + 0.5) + entity->shape.rotated_points[i].x;
        int point_world_y = (int)(entity->v_y + 0.5) + entity->shape.rotated_points[i].y;

        
        int screen_x = point_world_x - camera->x;
        int screen_y = point_world_y - camera->y;

        if (screen_x >= 0 && screen_x < camera->width &&
            screen_y >= 0 && screen_y < camera->height) {
            char ch;

            switch (entity->shape.texture) {
                case TEXTURE_SOLID:
                    ch = '^';
                    break;
                case TEXTURE_GRADIENT:
                    ch = get_char_for_distance(entity->shape.distances[i]);
                    break;
                case TEXTURE_FIRE:
                    ch = get_fire_char(i, entity->shape.distances[i], frame,
                                       entity->shape.original_points[i].y);
                    break;
                case  TEXTURE_SPEED:
                    float speed = sqrt(entity->transform.dx * entity->transform.dx +
                                       entity->transform.dy * entity->transform.dy);
                    float speed_factor = 1.0 + (speed * 0.1);
                    float adjusted_distance = entity->shape.distances[i] / speed_factor;
                    if (adjusted_distance < 1.0) ch = '#';
                    else if (adjusted_distance < 2.0) ch = '%';
                    else if (adjusted_distance < 4.0) ch = '*';
                    else if (adjusted_distance < 5.0) ch = '+';
                    else if (adjusted_distance < 6.0) ch = '.';
                    else ch = '-';
                default:
                    ch = '*';
            }
            buffer_draw_char(fb, screen_x + 1, screen_y + 1, ch, entity->color);
        }
    }
}

void render_entities (FrameBuffer *fb, Camera *camera, World *world, unsigned int frame) {
    GameObject * visible_entities[100];

    int entity_count = world_entity_in_region(world, camera->x, camera->y,
                                              camera->x + camera->width, camera->y + camera->height,
                                              visible_entities, 100);
    for (int i = 0;i < entity_count;i ++) {
        GameObject *entity = visible_entities[i];

        if (!world_is_visible(world, entity->cell_x, entity->cell_y)) {
            continue;
        }

        render_entity(fb, camera, visible_entities[i], frame);
    }
}

void render_terrain (FrameBuffer *fb, Camera *camera, World *world) {
    int sqr_size = 8;

    for (int screen_y = 0; screen_y < camera->height; screen_y++) {
        for (int screen_x = 0; screen_x < camera->width; screen_x++) {
            int world_x = camera->x + screen_x;
            int world_y = camera->y + screen_y;
            
            if (!world_is_visible(world, world_x, world_y)) {
                buffer_draw_char(fb, screen_x + 1, screen_y + 1, '.', COLOR_BLACK);
                continue;
            }

            float distance = world_get_distance_from_viewer(world, world_x, world_y,
                                                            world->viewer_x, world->viewer_y);

            TerrainType terrain = world_get_terrain(world, world_x, world_y);
            
            char ch;
            Color color;
            
            if (terrain == TERRAIN_WALL) {
                // Check if any neighbor is not a wall (border detection)
                bool is_border = 
                    world_get_terrain(world, world_x - 1, world_y) != TERRAIN_WALL ||
                    world_get_terrain(world, world_x + 1, world_y) != TERRAIN_WALL ||
                    world_get_terrain(world, world_x, world_y - 1) != TERRAIN_WALL ||
                    world_get_terrain(world, world_x, world_y + 1) != TERRAIN_WALL;
                
                if (is_border) {
                    ch = 'o';  // Border character
                    color = COLOR_BRIGHT_BLACK;
                } else {
                    ch = '#';  // Interior character
                    color = COLOR_BLACK;
                }
            } else if (terrain == TERRAIN_WATER){
                ch = '~';
                color = COLOR_BRIGHT_BLUE;
            } else {
                // This is the floor - keep the checkered pattern
                get_check(world_x, world_y, sqr_size, &ch, &color);
            }
            
            //gradient for FOV
            float fade_start = world->view_radius - 4.2f;
            float fade_end = world->view_radius;
            if (distance > fade_start) {
                float fade = (distance - fade_start) / (fade_end - fade_start);
                fade = fade < 0.0f ? 0.0f : (fade > 1.0f ? 1.0f : fade);
                if (fade > 0.75f) {
                    ch = '.';
                }
                else if (fade > 0.5f) {
                    color = COLOR_BRIGHT_BLACK;
                    ch = ':';
                }
                else if (fade > 0.5f) {
                    color = COLOR_BLACK;
                    ch = '*';
                }
            }

            buffer_draw_char(fb, screen_x + 1, screen_y + 1, ch, color);
        }
    }
}

void render_world(FrameBuffer *fb, World *world, Camera *camera, unsigned int frame) {

    init_frame_buffer(fb, camera->width, camera->height);
    render_terrain(fb, camera, world);
    render_entities(fb, camera, world, frame);
    present_frame(fb);
}

void draw_text (FrameBuffer *fb, int x, int y, const char *text, Color color) {
    int i = 0;
    while (text[i] != '\0') {
        buffer_draw_char(fb, x + i, y, text[i], color);
        i ++;
    }
}

void draw_text_centered(FrameBuffer *fb, int y, const char *text, Color color) {
    int len = 0;
    while (text[len] != '\0') len++;
    
    int x = (fb->width - len) / 2 + 1;  // +1 because buffer_draw_char uses 1-indexed
    draw_text(fb, x, y, text, color);
}

#define CAMERA_LERP_SPEED 0.7f

void camera_init(Camera *camera, int viewport_width, int viewport_height) {
    camera->x_float = 0.0f;
    camera->y_float = 0.0f;
    camera->x = 0;
    camera->y = 0;
    camera->width = viewport_width;
    camera->height = viewport_height;
}

void camera_follow_entity_smooth (Camera *camera, GameObject *entity, 
                                  int world_width, int world_height) {
    // Calculate where camera should be (centered on player)
    float desired_x = entity->v_x - camera->width / 2.0f;
    float desired_y = entity->v_y - camera->height / 2.0f;
    
    // Clamp to world boundaries
    if (desired_x < 0) desired_x = 0;
    if (desired_y < 0) desired_y = 0;
    if (desired_x + camera->width > world_width) {
        desired_x = world_width - camera->width;
    }
    if (desired_y + camera->height > world_height) {
        desired_y = world_height - camera->height;
    }
    
    // Smoothly interpolate camera toward desired position
    float dx = desired_x - camera->x_float;
    float dy = desired_y - camera->y_float;
    
    camera->x_float += dx * CAMERA_LERP_SPEED;
    camera->y_float += dy * CAMERA_LERP_SPEED;
    
    // Snap when very close to avoid infinite creep
    if (fabs(dx) < 0.1f) camera->x_float = desired_x;
    if (fabs(dy) < 0.1f) camera->y_float = desired_y;
    
    // Convert to integers for rendering
    camera->x = (int)(camera->x_float + 0.5);
    camera->y = (int)(camera->y_float + 0.5);
}

void camera_follow_entity (Camera *camera, GameObject *entity, int world_width, int world_height) {
    camera->x = (int)entity->v_x - camera->width / 2;
    camera->y = (int)entity->v_y - camera->height / 2;
    
    if (camera->x < 0) camera->x = 0;
    if (camera->y < 0) camera->y = 0;
    if (camera->x + camera->width > world_width) {
        camera->x = world_width - camera->width;
    }
    if (camera->y + camera->height > world_height) {
        camera->y = world_height - camera->height;
    }
}

void camera_world_to_screen(Camera *camera, int world_x, int world_y, int *screen_x, int *screen_y) {
    *screen_x = world_x - camera->x;
    *screen_y = world_y - camera->y;
}

void camera_screen_to_world(Camera *camera, int screen_x, int screen_y, int *world_x, int *world_y) {
    *world_x = screen_x + camera->x;
    *world_y = screen_y + camera->y;
}

bool camera_is_visible(Camera *camera, int world_x, int world_y) {
    return world_x >= camera->x && 
           world_x < camera->x + camera->width &&
           world_y >= camera->y && 
           world_y < camera->y + camera->height;
}

