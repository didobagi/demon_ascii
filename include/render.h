#ifndef RENDER_H
#define RENDER_H

#include "types.h"

void render_text (const char *text, int x, int y);

void render_text_centered (const char *text, int term_w, int term_h, int y_offset);

void render_char(char ch, int x, int y);

void render (GameObject *obj, float center_x, float center_y,
             bool erase, unsigned int frame);

void render_enemies (Enemy *enemies, int count, int screen_w, int screen_h, unsigned int frame);
void render_background (int max_x, int max_y, unsigned int frame,
                Sector sectors[SECTOR_ROWS][SECTOR_COLS]);

void render_collectibles (CollectiblePoint *collectibles, int count, unsigned int frame);

#endif
