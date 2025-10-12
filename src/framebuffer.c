#include "../include/frame_buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
    static char output_buffer[4000000];
    int pos = 0;

    Color current_color = COLOR_BLACK;

        static const char* COLOR_CODES[] = {
        "\033[30m", "\033[31m", "\033[32m", "\033[33m",
        "\033[34m", "\033[35m", "\033[36m", "\033[37m",
        "\033[90m", "\033[91m", "\033[92m", "\033[93m",
        "\033[94m", "\033[95m", "\033[96m", "\033[97m"
    };

    for (int y = 0; y < fb->height;y ++) {
        pos += sprintf(output_buffer + pos, "\033[%d;1H", y + 1);
        for (int x = 0; x < fb->width;x ++) {
            Color cell_color = fb->colors[y][x];

            if (cell_color != current_color) {
               pos += sprintf(output_buffer + pos,"%s", COLOR_CODES[cell_color]);
               current_color = cell_color;
            }
            output_buffer[pos++] = fb->cells[y][x];
        }
    }
    pos += sprintf(output_buffer + pos, "\033[0m");
    write(STDOUT_FILENO, output_buffer, pos);
}
