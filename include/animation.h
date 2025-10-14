#ifndef ANIMATION_H
#define ANIMATION_H

#include "types.h"

void animation_set (GameObject *entity, Point **frames,
                    int *frame_counts, int total_frames, float speed);
void animation_update (GameObject *entity, float delta_time);

#endif
