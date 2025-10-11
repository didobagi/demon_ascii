#include "../include/morph.h"
#include "../include/game.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MORPH_DURATION_FRAMES 30

typedef struct MorphState {
    Point source_points[100];
    Point target_points[100];
    Point morphing_points[100];
    int point_count;
    unsigned int start_frame;
    bool morphing_to_snake;
} MorphState;

static MorphState active_morph = {0};
static bool morph_in_progress = false;

void setup_morph_forms(GameObject *obj, Point *form_a, int count_a,
                       Point *form_b, int count_b) {
    obj->demon_form_template = form_a;
    obj->demon_form_point_count = count_a;
    obj->snake_form_template = form_b;
    obj->snake_form_point_count = count_b;

    obj->in_snake_form = false;
    obj->is_morphing = false;

    for (int i = 0;i < 100;i ++) {
        obj->point_collected[i] = true;
    }
    obj->total_collected_count = count_a;

    obj->shape.original_points = form_a;
    obj->shape.point_count = count_a;
}

void initiate_morph(GameObject *obj) {
    if (obj->is_morphing) {
        return;
    }
    Point *source_template;
    Point *target_template;
    int point_count;
    bool morphing_to_snake;

    if(obj->in_snake_form) {
        source_template = obj->snake_form_template;
        target_template = obj->demon_form_template;
        point_count = obj->demon_form_point_count;
        morphing_to_snake = false;
    } else {
        source_template = obj->demon_form_template;
        target_template = obj->snake_form_template;
        point_count = obj->snake_form_point_count;
        morphing_to_snake = true;

        for (int i = CORE_SNAKE_POINTS;i < point_count;i ++) {
            obj->point_collected[i] = false;
        }
        obj->total_collected_count = CORE_SNAKE_POINTS;
    }
    for (int i = 0;i < point_count;i ++) {
        active_morph.source_points[i] = source_template[i];
        active_morph.target_points[i] = target_template[i];
        active_morph.morphing_points[i] = source_template[i];
    }
    active_morph.point_count = point_count;
    active_morph.morphing_to_snake = morphing_to_snake;
    active_morph.start_frame = 0;

    obj->shape.original_points = active_morph.morphing_points;
    obj->shape.point_count = point_count;

    obj->is_morphing = true;
    morph_in_progress = true;
}

static void spawn_collectible_for_morph (GameObject *obj, GameState *game,
                                         int screen_w, int screen_h) {
    game->collectible_count =0;
    int collectible_index = 0;
    for (int shape_index = CORE_SNAKE_POINTS;shape_index < obj->snake_form_point_count;shape_index ++) {
        CollectiblePoint *cp = &game->collectibles[collectible_index];

        int margin = 5; //leave so not right on the screen

        cp->x = margin + ((float)rand() / RAND_MAX) * (screen_w - 2 * margin);
        cp->y = margin + ((float)rand() / RAND_MAX) * (screen_h - 2 * margin);

        cp->shape_point_index = shape_index;
        cp->active = true;

        //TODO drift

        collectible_index ++;
    }
    game->collectible_count = collectible_index;
}

bool update_morph(GameObject *obj, GameState *game, unsigned int frame) {
    if (!obj->is_morphing) {
        return false;
    }

    if (active_morph.start_frame == 0) {
        active_morph.start_frame = frame;
    }
    
    unsigned int frame_elapsed = frame - active_morph.start_frame;
    float progress = (float)frame_elapsed / (float)MORPH_DURATION_FRAMES;
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    //lerp!!!  l(a,b,t) = a + t * (b-a)
    for (int i = 0;i < active_morph.point_count;i ++) {
        float dx = active_morph.target_points[i].x - active_morph.source_points[i].x;
        float dy = active_morph.target_points[i].y - active_morph.source_points[i].y;

        active_morph.morphing_points[i].x = active_morph.source_points[i].x + (int)(progress * dx);
        active_morph.morphing_points[i].y = active_morph.source_points[i].y + (int)(progress * dy);
    }

    obj->bounds = calculate_shape_bounds(obj->shape.original_points,
                                         obj->shape.point_count);
    update_visual(obj);

    if (progress >= 1.0f) {
        obj->is_morphing = false;
        morph_in_progress = false;
        obj->in_snake_form = !obj->in_snake_form;

        if (obj->in_snake_form) {
            obj->shape.original_points = obj->snake_form_template;
            spawn_collectible_for_morph(obj,game, game->max_x, game->max_y);
        } else {
            obj->shape.original_points = obj->demon_form_template;
        }
        return false;
    }
    return true;
}


