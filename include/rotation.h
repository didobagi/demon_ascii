#ifndef ROTATION_H
#define ROTATION_H

#include "types.h"

void rotate_shape(Shape *shape, float angle_radians);
float angle_toward(int from_x, int from_y, int to_x, int to_y);
void update_entity_facing(GameObject *entity, int dx, int dy);
void apply_facing_to_shape(GameObject *entity);

#endif
