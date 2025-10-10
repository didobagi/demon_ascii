#include "render.h"
#include <stdio.h>
#include <math.h>

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

void render_background(int max_x, int max_y, unsigned int frame,
                        Sector sectors[SECTOR_ROWS][SECTOR_COLS]) {
    int sector_w = max_x / SECTOR_COLS;
    int sector_h = max_y / SECTOR_ROWS;

    for (int y = 1; y <= max_y; y++) {
        printf("\033[%d;1H", y); 
        for (int x = 1; x <= max_x; x++) {
            int sector_x = (x - 1) / sector_w;
            int sector_y = (y - 1) / sector_h;

            if (sector_x >= SECTOR_COLS) sector_x = SECTOR_COLS - 1;
            if (sector_y >= SECTOR_ROWS) sector_y = SECTOR_ROWS - 1;
            
            char ch;
            if (sectors[sector_y][sector_x].is_dangerous) {
                int sector_start_y = sector_y * sector_h + 1;
                int y_in_sector = y - sector_start_y;
                float y_ratio = (float)y_in_sector / (float)sector_h;

                ch = get_sector_fire_char(x, y, y_ratio, frame);
            } else {
                ch = 'A';
            }
            putchar(ch);
        }
    }
    fflush(stdout);
}

void render (GameObject *obj, float center_x, float center_y, bool erase, unsigned int frame) {
    char buffer[2000];
    int pos = 0;
    for (int i = 0;i < obj->shape.point_count;i ++) {
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
        pos += sprintf(buffer + pos, "\033[%d;%dH", draw_y, draw_x);
        pos += sprintf(buffer + pos, "%c",ch);
    }
    printf("%s", buffer);
    fflush(stdout);
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
