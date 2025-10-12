#include "../include/render.h"
#include "../include/frame_buffer.h"
#include <stdio.h>
#include <math.h>

static const char* COLOR_CODES[] = {
    "\033[30m",  // BLACK
    "\033[31m",  // RED
    "\033[32m",  // GREEN
    "\033[33m",  // YELLOW
    "\033[34m",  // BLUE
    "\033[35m",  // MAGENTA
    "\033[36m",  // CYAN
    "\033[37m",  // WHITE
    "\033[90m",  // BRIGHT_BLACK (dark grey)
    "\033[91m",  // BRIGHT_RED
    "\033[92m",  // BRIGHT_GREEN
    "\033[93m",  // BRIGHT_YELLOW
    "\033[94m",  // BRIGHT_BLUE
    "\033[95m",  // BRIGHT_MAGENTA
    "\033[96m",  // BRIGHT_CYAN
    "\033[97m"   // BRIGHT_WHITE
};

char get_char_for_distance (float distance) {
    if (distance < 2.0) return '*';
    if (distance < 4.0) return '+';
    if (distance < 6.0) return '=';
    return '-';
}

char get_char_for_distance_and_speed (float distance, float speed) {

    float speed_factor = 1.0 + (speed * 0.1);
    float adjusted_distance = distance/speed_factor;
    if (adjusted_distance < 1.0) return '#';
    if (adjusted_distance < 2.0) return '%';
    if (adjusted_distance < 4.0) return '*';
    if (adjusted_distance < 5.0) return '+';
    if (adjusted_distance < 6.0) return '.';
    return '-';
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

    if (cycle < 10) return '#';
    if (cycle < 20) return '@';
    if (cycle < 30) return '*';
    if (cycle < 35) return '+';
    return '.';
    }
    float transtion = (point_y - fire_tresh)/(solid_tresh - fire_tresh);
    int phase = (point_index * 7 + (int)(distance * 3)) % 20;
    int cycle_lengt = 40 + (int)(transtion*60);
    int cycle = (frame + phase) % 40;

    int brigth_du = (int)(10 * (1.0 - transtion));
    if (cycle < brigth_du) return '#';
    if (cycle < brigth_du * 2) return '@';
    return '#';
}

char get_sector_fire_char (int x, int y, float y_ratio, unsigned int frame) {
    float flame_ar = 0.6;
    float base_ar = 0.7;
    
    if(y_ratio > base_ar) {
        return 'B';
    }
    if (y_ratio < flame_ar) {
    int phase = (x * 7 + y * 3) % 20;
    int cycle = (frame + phase) % 40;

    if (cycle < 10) return '#';
    if (cycle < 20) return '@';
    if (cycle < 30) return '*';
    if (cycle < 35) return '+';
    return '.';
    }
    float transtion = (y_ratio - flame_ar)/(base_ar - flame_ar);
    int phase = (x * 7 + y * 3) % 20;
    int cycle_lengt = 40 + (int)(transtion*60);
    int cycle = (frame + phase) % cycle_lengt;

    int brigth_du = (int)(10 * (1.0 - transtion));
    if (cycle < brigth_du) return '#';
    if (cycle < brigth_du * 2) return '@';
    return 'B';
}

void render_background(FrameBuffer *fb, int max_x, int max_y, unsigned int frame,
        Sector sectors[SECTOR_ROWS][SECTOR_COLS]) {
    int sector_w = max_x / SECTOR_COLS;
    int sector_h = max_y / SECTOR_ROWS;

    int current_sector_x = -1;
    int current_sector_y = -1;
    bool current_is_dangerous = false;
    Color current_color = COLOR_BRIGHT_BLACK;

    for (int y = 1; y <= max_y; y++) {
        for (int x = 1; x <= max_x; x++) {
            int sector_x = (x - 1) / sector_w;
            int sector_y = (y - 1) / sector_h;

            if (sector_x >= SECTOR_COLS) sector_x = SECTOR_COLS - 1;
            if (sector_y >= SECTOR_ROWS) sector_y = SECTOR_ROWS - 1;

            if (sector_x != current_sector_x || sector_y != current_sector_y) {
                current_sector_x = sector_x;
                current_sector_y = sector_y;
                current_is_dangerous = sectors[sector_y][sector_x].is_dangerous;

                current_color = current_is_dangerous ? COLOR_BRIGHT_RED : COLOR_BRIGHT_BLACK;
            }   
            char ch;
            if (current_is_dangerous) {
                int sector_start_y = sector_y * sector_h + 1;
                int y_in_sector = y - sector_start_y;
                float y_ratio = (float)y_in_sector / (float)sector_h;
                ch = get_sector_fire_char(x, y, y_ratio, frame);
            } else {
                ch = 'A';
            }
            buffer_draw_char(fb, x, y, ch, current_color);
        }
    }
}


void render (FrameBuffer *fb, GameObject *obj, float center_x, float center_y,
             bool erase, unsigned int frame) {

    for (int i = 0;i < obj->shape.point_count;i ++) {

        if (obj->in_snake_form && !obj->point_collected[i]) {
            continue;
        }

        int draw_x = (int)center_x + obj->shape.rotated_points[i].x;
        int draw_y = (int)center_y + obj->shape.rotated_points[i].y;
        char ch;
        if (erase) {
            ch = ' ';
        } else {
            switch (obj->shape.texture) {
                case TEXTURE_SOLID:
                    ch = '*';
                    break;
                case TEXTURE_GRADIENT:
                    ch = get_char_for_distance(obj->shape.distances[i]);
                    break;
                case TEXTURE_FIRE:
                    ch = get_fire_char(i, obj->shape.distances[i], frame,
                            obj->shape.original_points[i].y);
                    break;
                case TEXTURE_SPEED:
                    float speed = sqrt(obj->transform.dx * obj->transform.dx +
                            obj->transform.dy * obj->transform.dy);
                    ch = get_char_for_distance_and_speed(obj->shape.distances[i], speed);
                    break;
                default:
                    ch = '*';
            }
        }
        Color draw_color = erase ? COLOR_BLACK : obj->color;
        buffer_draw_char(fb, draw_x, draw_y, ch, draw_color);
    }
}

void render_text(const char *text, int x, int y) {
    printf("\033[%d;%dH", y, x);
    printf("%s", text);
    fflush(stdout);
}

void render_text_centered(const char *text, int term_w, int term_h, int y_offset) {
    int text_len = 0;
    for (const char *p = text; *p != '\0'; p++) {
        text_len++;
    }
    
    int center_x = term_w / 2;
    int center_y = term_h / 2;
    int text_x = center_x - (text_len / 2);
    int text_y = center_y + y_offset;
    
    render_text(text, text_x, text_y);
}

void render_char(char ch, int x, int y) {
    printf("\033[%d;%dH", y, x);
    putchar(ch);
    fflush(stdout);
}

void render_collectibles (FrameBuffer *fb, CollectiblePoint * collectibles,
                          int count, unsigned int frame) {   
    
    const char anim_chars[] = {'!', '?', '.', '+'};
    const int anim_chars_count = 4;

    for (int i  = 0;i < count;i ++) {
        CollectiblePoint *cp = &collectibles[i];

        if(!cp->active) {
            continue;
        }

        //TODO apply drift
        int animation_phase = (frame + i * 7) / 10;
        int char_index = animation_phase % anim_chars_count;
        char ch = anim_chars[char_index];

        float render_x = cp->x;
        float render_y = cp->y;
        
        buffer_draw_char(fb, (int)render_x, (int)render_y, ch, COLOR_BRIGHT_YELLOW);
    }
}

void render_enemies (FrameBuffer *fb, Enemy *enemies, int count,
                     int screen_w, int screen_h, unsigned int frame) {
    for (int i = 0;i < count;i ++) {
        Enemy *enemy = &enemies[i];

        if(!enemy->active) {
            continue;
        }
        
        for (int j = 0;j < enemy->point_count;j ++) {
            int draw_x = (int)enemy->x + enemy->rotated_points[j].x;
            int draw_y = (int)enemy->y + enemy->rotated_points[j].y;

            if (draw_x < 1 || draw_x >= screen_w || draw_y < 1 || draw_y >= screen_h) {
                continue;
            }
            buffer_draw_char(fb, draw_x, draw_y, '@', COLOR_CYAN);
        }
    }
}
