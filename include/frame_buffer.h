#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "types.h"

typedef struct {
    char cells[200][500];
    Color colors[200][500];
    int width;
    int height;
} FrameBuffer;

void init_frame_buffer (FrameBuffer *fb, int width, int height);

void buffer_draw_char (FrameBuffer *fb, int x, int y, char c, Color color);

void present_frame (FrameBuffer *fb);

#endif
