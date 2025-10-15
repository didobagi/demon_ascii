#include "../include/render.h"
#include "../include/world.h"
#include "../include/frame_buffer.h"
#include "../include/camera.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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
            } else {
                // This is the floor - keep the checkered pattern
                get_check(world_x, world_y, sqr_size, &ch, &color);
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
