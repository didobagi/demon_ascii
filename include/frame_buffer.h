#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "types.h"

#define MAX_WIDTH 500
#define MAX_HEIGHT 200

typedef struct {
    int width;
    int height;
    char cells[MAX_HEIGHT][MAX_WIDTH];
    Color colors[MAX_HEIGHT][MAX_WIDTH];

    char prev_cells[MAX_HEIGHT][MAX_WIDTH];
    Color prev_colors[MAX_HEIGHT][MAX_WIDTH];
} FrameBuffer;
void init_frame_buffer (FrameBuffer *fb, int width, int height);

void buffer_draw_char (FrameBuffer *fb, int x, int y, char c, Color color);

void present_frame (FrameBuffer *fb);

#endif
