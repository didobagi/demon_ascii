#include "../include/game.h"
#include "../include/types.h"
#include <math.h>
#include <stdlib.h>

void update_transform (GameObject *obj, int max_x, int max_y) {
    int min = 4;
    int max = 7;
    int rand_n = min + rand() % (max - min + 1);
    int speed_y = rand_n/4;
    int speed_x = 1 + abs((int)obj->transform.y - max_y/2) / rand_n;
    obj->transform.new_x = obj->transform.x + speed_x * obj->transform.dx;
    obj->transform.new_y = obj->transform.y + speed_y * obj->transform.dy;
 
    obj->transform.angle = atan2(obj->transform.dy, obj->transform.dx);
}

void update_visual (GameObject *obj) {
    float angle = obj->transform.angle;

    for (int i = 0;i < obj->shape.point_count;i ++) {
        float x = obj->shape.original_points[i].x;
        float y = obj->shape.original_points[i].y;

        obj->shape.rotated_points[i].x = (int)(x * cos(angle) - y * sin(angle));
        obj->shape.rotated_points[i].y = (int)(x * sin(angle) + y * cos(angle));
    }
}

bool check_boundaries (GameObject *obj, int max_x, int max_y) {
    float radius = obj->collision.max_radius;
    bool collision = false;

    if (obj->transform.new_x - radius <= 0 ||
        obj->transform.new_x + radius >= max_x) {
        obj->transform.dx = -obj->transform.dx;
        collision = true;
    }
     if (obj->transform.new_y - radius <= 0 ||
        obj->transform.new_y + radius >= max_y) {
        obj->transform.dy = -obj->transform.dy;
        collision =  true;
    }
    return collision;
}

bool check_obj_collision (GameObject *obj_a, GameObject *obj_b) {
    float dx = obj_a->transform.new_x - obj_b->transform.x;
    float dy = obj_a->transform.new_y - obj_b->transform.y;
    float dist_sqr = dx*dx + dy*dy;
    float rad_sum = obj_a->collision.max_radius + obj_b->collision.max_radius;

    if (dist_sqr < rad_sum*rad_sum) {
        obj_a->transform.dx = -obj_a->transform.dx;
        obj_a->transform.dy = -obj_a->transform.dy;
        return true;
    }
    return false;
}

void commit_transf (GameObject *obj) {
    obj->transform.x = obj->transform.new_x;
    obj->transform.y = obj->transform.new_y;
}

ShapeBounds calculate_shape_bounds (Point *points, int count) {
    ShapeBounds bounds = {
        .min_x = points[0].x,
        .max_x = points[0].x,
        .min_y = points[0].y,
        .max_y = points[0].y
    };

    for (int i = 1;i < count;i ++) {
        if (points[i].x < bounds.min_x) bounds.min_x = points[i].x;
        if (points[i].x > bounds.max_x) bounds.max_x = points[i].x;
        if (points[i].y < bounds.min_y) bounds.min_y = points[i].y;
        if (points[i].y > bounds.max_y) bounds.max_y = points[i].y;
    }

    return bounds;
}

ShapeBounds calculate_shape_bounds_selective (Point *points, int count,
                                    bool *collected, bool in_snake_form) {
    int first_collected = -1;
    for (int i = 0; i < count; i++) {
        if (!in_snake_form || collected[i]) {
            first_collected = i;
            break;
        }
    }
    
    if (first_collected == -1) {
        return (ShapeBounds){0, 0, 0, 0};
    }

    ShapeBounds bounds = {
        .min_x = points[first_collected].x,
        .max_x = points[first_collected].x,
        .min_y = points[first_collected].y,
        .max_y = points[first_collected].y
    };

    for (int i = first_collected + 1;i < count;i ++) {
        if (!in_snake_form || collected[i]) {
            if (points[i].x < bounds.min_x) bounds.min_x = points[i].x;
            if (points[i].x > bounds.max_x) bounds.max_x = points[i].x;
            if (points[i].y < bounds.min_y) bounds.min_y = points[i].y;
            if (points[i].y > bounds.max_y) bounds.max_y = points[i].y;
        }
    }

    return bounds;
}

int create_object(GameObject objects[], int* count,
        Point *shape_template, int point_count,
        float x, float y, float dx, float dy,
        float collision_radius, TextureType texture,
        Color color) {
    if(*count >= MAX_OBJECTS) {
        return -1;
    }
    static Point rotated_buffers[MAX_OBJECTS][1000];

    GameObject *obj = &objects[*count];

    obj->transform.x = x;
    obj->transform.y = y;
    obj->transform.angle = atan2(dy, dx);
    obj->transform.dx = dx;
    obj->transform.dy = dy;
    obj->color = color;

    obj->shape.original_points = shape_template;
    obj->shape.rotated_points = rotated_buffers[*count];
    obj->shape.point_count = point_count;

    obj->bounds = calculate_shape_bounds(shape_template, point_count);
    obj->shape.display_char = '*';
    static float distance_buffers[MAX_OBJECTS][1000];
    obj->shape.distances = distance_buffers[*count];

    for (int i = 0;i < point_count;i ++) {
        float x = shape_template[i].x;
        float y = shape_template[i].y;
        obj->shape.distances[i] = sqrt(x*x + y*y);
    }
    obj->shape.texture = texture;

    static CollisionCircle collsion_buffers[MAX_OBJECTS];
    collsion_buffers[*count].x = 0;
    collsion_buffers[*count].y = 0;
    collsion_buffers[*count].radius = collision_radius;

    obj->collision.circles = &collsion_buffers[*count];
    obj->collision.circle_count = 1;
    obj->collision.max_radius = collision_radius;

    obj->active = true;

    int index = *count;
    (*count)++;
    update_visual(obj);

    return index;
}

void bounce (GameObject *obj, int screen_w, int screen_h) {
    float left_edge = obj->transform.x + obj->bounds.min_x;
    float right_edge = obj->transform.x + obj->bounds.max_x;
    float top_edge = obj->transform.y + obj->bounds.min_y;
    float bottom_edge = obj->transform.y + obj->bounds.max_y;

    if (left_edge < 0) {
        obj->transform.x -= left_edge;
    }
    if (right_edge >= screen_w) {
        obj->transform.x -= (right_edge - screen_w + 1);
    }
    if (top_edge < 1) {
        obj->transform.y -= (top_edge - 1);
    }
    if (bottom_edge >= screen_h) {
        obj->transform.y -= (bottom_edge - screen_h + 1);
    }
}

void collect_point (GameObject *player, CollectiblePoint *cp, GameState *game) {
    cp->active = false;

    int shape_index = cp->shape_point_index;
    player->point_collected[shape_index] = true;
    
    player->total_collected_count++;

    player->bounds = calculate_shape_bounds_selective(
            player->shape.original_points,
            player->shape.point_count,
            player->point_collected,
            player->in_snake_form);

    //TODO add feedback
}

void check_collectible_collision (GameObject *player, GameState *game) {
    //avoid sqrt by comparing sq values
    const float collection_radius = 8.0f;
    const float collection_radius_sq = collection_radius * collection_radius;\

    for (int i  = 0;i < game->collectible_count;i ++) {
        CollectiblePoint *cp = &game->collectibles[i];
        if(!cp->active) {
            continue;
        }

        float dx = player->transform.x - cp->x;
        float dy = player->transform.y - cp->y;
        float distance_sq = dx * dx + dy * dy;

        if (distance_sq < collection_radius_sq) {
            collect_point(player, cp, game);
        }
    }
}
