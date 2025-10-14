#ifndef SHAPES_H
#define SHAPES_H

#include "types.h"

extern Point carachter_test_template[];
extern Point ghost_template[];
extern Point demon_template[];
extern Point demon_snake_template[];
extern Point monkey_template[];
extern Point eye_template[];
extern Point tower_template[];

extern const int ghost_point_count;
extern const int demon_point_count;
extern const int demon_snake_point_count;
extern const int monkey_point_count;
extern const int eye_point_count;
extern const int carachter_test_point_count;
extern const int tower_point_count;

extern Point* carachter_breathe_frames[];
extern int carachter_breathe_frame_counts[];
extern const int carachter_breathe_total_frames;

extern Point* monkey_walk_frames[];
extern int monkey_walk_frame_counts[];
extern const int monkey_walk_total_frames;

extern Point* monkey_idle_frames[];
extern int monkey_idle_frame_counts[];
extern const int monkey_idle_total_frames;

#endif
