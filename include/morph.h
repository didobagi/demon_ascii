#ifndef MORPH_H
#define MORPH_H

#include "types.h"

void setup_morph_forms (GameObject *obj, Point *form_a, int count_a,
                        Point *form_b, int count_b);

void  initiate_morph (GameObject *obj);

bool update_morph (GameObject *obj, GameState *game, unsigned int frame);

#endif
