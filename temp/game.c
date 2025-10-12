#include "../include/game.h"
#include "../include/types.h"
#include <math.h>
#include <stdlib.h>

#define DETECTION_RANGE 30.0f
#define DEACTIVATION_RANGE 50.0f
#define ALERT_DURATION 60
#define IDLE_DUR_BEFORE_WAND 79

float calculate_distance (Enemy *enemy, GameObject *player) {
    float dx = player->transform.x - enemy->x;
    float dy = player->transform.y - enemy->y;
    return sqrt(dx*dx + dy*dy);
}

void change_enemy_state (Enemy *enemy, AIState new_state, unsigned int frame) {
    enemy->current_state = new_state;
    enemy->state_entered_frame = frame;
}

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

void spawn_enemy (GameState *game, Point *shape_template, int point_count,
                  float x, float y, float speed) {
    Enemy *enemy = NULL;
    for (int i = 0;i < game->enemy_count;i ++) {
        if (!game->enemies[i].active) {
            enemy = &game->enemies[i];
            break;
        }
    }

    if (enemy == NULL && game->enemy_count < 10) {
        enemy = &game->enemies[game->enemy_count];
        game->enemy_count ++;
    }

    if (enemy == NULL) {
        return;
    }

    //init enemy
    enemy->x = x;
    enemy->y = y;
    enemy->dx = 0.0f;
    enemy->dy = 0.0f;
    enemy->shape_template = shape_template;
    enemy->point_count = point_count;
    enemy->angle = 0.0f;
    enemy->speed = speed;
    enemy->active = true;

    enemy->current_state = AI_STATE_IDLE;
    enemy->state_entered_frame = 0;
    enemy->health = 100;
    enemy->max_health = 100;

    enemy->bounds = calculate_shape_bounds(shape_template, point_count);

    for (int i = 0;i < point_count;i ++) {
        enemy->rotated_points[i] = shape_template[i];
    }
}

void enemy_seek (Enemy *enemy, GameObject *player) {
    float to_player_x = player->transform.x - enemy->x;
    float to_player_y = player->transform.y - enemy->y;

    float distance = sqrt(to_player_x * to_player_x + to_player_y * to_player_y);
    if (distance < 1.0f) {
        enemy->dx = 0.0f;
        enemy->dy = 0.0f;
        return;
    }
    //normalize
    enemy->dx = (to_player_x / distance) *enemy->speed;
    enemy->dy = (to_player_y / distance) *enemy->speed;

    enemy->angle = atan2(enemy->dy, enemy->dx);
}

void enemy_flee (Enemy *enemy, GameObject *player) {
    float away_from_player_x =  enemy->x - player->transform.x;
    float away_from_player_y =  enemy->y - player->transform.y;

    float distance = sqrt(away_from_player_x * away_from_player_x + 
                          away_from_player_y * away_from_player_y);
    if (distance < 1.0f) {
        enemy->dx = enemy->speed;
        enemy->dy = 0.0f;
        return;
    }
    //normalize
    enemy->dx = (away_from_player_x / distance) *enemy->speed;
    enemy->dy = (away_from_player_y / distance) *enemy->speed;

    enemy->angle = atan2(enemy->dy, enemy->dx);
}

void enemy_idle (Enemy *enemy) {
    enemy->dx = 0;
    enemy->dy = 0;
}

void enemy_wonder (Enemy *enemy, unsigned int frame) {
    if (frame % 60 == 0) {
        float random_angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
        enemy->dx = cos(random_angle) * enemy->speed * 0.6f;
        enemy->dy = sin(random_angle) * enemy->speed * 0.6f;
        enemy->angle = random_angle;
    }
}

void check_enemy_state_transitions (Enemy *enemy, GameObject *player, unsigned int frame) {
    float distance_to_player = calculate_distance (enemy, player);

    switch (enemy->current_state) {
        case AI_STATE_IDLE:
            if (distance_to_player < DETECTION_RANGE) {
                if  (player->in_snake_form) {
                    change_enemy_state (enemy, AI_STATE_ALERT, frame);
                } else {
                    change_enemy_state(enemy, AI_STATE_FLEE, frame);
                }
            }
            else if (frame - enemy->state_entered_frame > IDLE_DUR_BEFORE_WAND) {
                change_enemy_state(enemy, AI_STATE_WANDER, frame);
            }

            break;
        case AI_STATE_ALERT:
            if (frame - enemy->state_entered_frame > ALERT_DURATION) {
                if (player->in_snake_form) {
                    change_enemy_state (enemy,AI_STATE_PURSUE, frame);
                } else {
                    change_enemy_state(enemy, AI_STATE_FLEE, frame);
                }
            }
            if (distance_to_player > DETECTION_RANGE * 1.5) {
                change_enemy_state (enemy, AI_STATE_IDLE, frame);
            }
            break;
        case AI_STATE_PURSUE:
            if (!player->in_snake_form) {
                change_enemy_state(enemy, AI_STATE_FLEE, frame);
            }

            if (enemy->health < enemy->max_health * 0.3) {
                change_enemy_state (enemy, AI_STATE_FLEE, frame);
            }
            if (distance_to_player > DEACTIVATION_RANGE) {
                change_enemy_state (enemy, AI_STATE_IDLE, frame);
            }
            break;
        case AI_STATE_FLEE:
            if (player->in_snake_form) {
                change_enemy_state(enemy, AI_STATE_PURSUE, frame);
            }
            //if (enemy->health >  enemy->max_health * 0.5) {
            //    change_enemy_state (enemy, AI_STATE_PURSUE, frame);
            //}
            break;
        case AI_STATE_WANDER:
            if (distance_to_player < DETECTION_RANGE) {
                if (player->in_snake_form) {
                    change_enemy_state(enemy, AI_STATE_ALERT, frame);
                } else {
                    change_enemy_state(enemy, AI_STATE_FLEE, frame);
                }
            }
            break;
    }
}

void execute_enemy_state_behaviour (Enemy *enemy, GameObject *player, unsigned int frame) {
    switch (enemy->current_state) {
        case AI_STATE_IDLE:
            enemy_idle(enemy);
            break;
        case AI_STATE_ALERT:
            enemy_idle(enemy);
            break;
        case AI_STATE_PURSUE:
            enemy_seek(enemy, player);
            break;
        case AI_STATE_FLEE:
            enemy_flee (enemy, player);
            break;
        case AI_STATE_WANDER:
            enemy_wonder(enemy, frame);
            break;
    }
    enemy->x += enemy->dx;
    enemy->y += enemy->dy;
}
void update_enemy (Enemy *enemy, GameObject *player, unsigned int frame) {
    check_enemy_state_transitions (enemy, player, frame);

    execute_enemy_state_behaviour (enemy, player, frame);
}



void update_enemies (GameState *game) {
    GameObject *player = &game->objects[0];

    for (int i = 0;i < game->enemy_count;i ++) {
        Enemy *enemy = &game->enemies[i];
    
        if(!enemy->active) {
            continue;
        }

        update_enemy(enemy, player, game->frame);
        
        float angle = enemy->angle;
        for (int j = 0;j < enemy->point_count;j ++) {
            float x = enemy->shape_template[j].x;
            float y = enemy->shape_template[j].y;

            enemy->rotated_points[j].x = (int)(x * cos(angle) - y * sin(angle));
            enemy->rotated_points[j].y = (int)(x * sin(angle) + y * cos(angle));
        }
    }
}
