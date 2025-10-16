#include "../include/enemy_ai.h"
#include "../include/movement.h"
#include "../include/rotation.h"
#include <math.h>
#include <stdlib.h>

#define ALERT_DISTANCE 15.0f
#define ALERT_DURATION 3.0f
#define WANDER_MOVE_INTERVAL 0.5f

static float distance_between (int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return sqrt((float)(dx*dx + dy*dy));
}

static void find_direction_toward (int from_x, int from_y,
                                   int to_x, int to_y,
                                   int *out_dx, int *out_dy) {

    int dx = to_x - from_x;
    int dy = to_y - from_y;

    if (dx > 0) *out_dx = 1;
    else if (dx < 0) *out_dx = -1;
    else *out_dx = 0;
    if (dy > 0) *out_dy = 1;
    else if (dy < 0) *out_dy = -1;
    else *out_dy = 0;
}

bool enemy_try_move_direction (GameObject *enemy, World *world, int dx, int dy) {


    if (dx == 0 && dy == 0) {
        return false;
    }

    update_entity_facing(enemy, dx, dy);
    if (movement_try_mov(enemy, world, dx, dy)) {
        return true;
    }

    //path blocked

    if (dx != 0) {
        update_entity_facing(enemy, dx, 0);
        if (movement_try_mov(enemy, world, dx, 0)) {
            return true;
        }
    }
    if (dy != 0) {
        update_entity_facing(enemy, 0, dy);
        if (movement_try_mov(enemy, world, 0, dy)) {
            return true;
        }
    }

    if (dx != 0) {
        update_entity_facing(enemy, 0, 1);
        if (movement_try_mov(enemy, world, 0, 1)) {
            return true;
        }
        update_entity_facing(enemy, 0, -1);
        if (movement_try_mov(enemy, world, 0, -1)) {
            return true;
        }
    }
    if (dy != 0) {
        update_entity_facing(enemy, 1, 0);
        if (movement_try_mov(enemy, world, 1, 0)) {
            return true;
        }
        update_entity_facing(enemy, -1, 0);
        if (movement_try_mov(enemy, world, -1, 0)) {
            return true;
        }
    }

    return false;
}

bool enemy_try_move_toward (GameObject *enemy, int target_x, int target_y, World *world) {
    int dx, dy;
    find_direction_toward(enemy->cell_x, enemy->cell_y, target_x, target_y, &dx, &dy);
    return enemy_try_move_direction(enemy, world, dx, dy);
}

bool enemy_try_flee_from(GameObject *enemy, int target_x, int target_y, World *world) {
    int dx, dy;
    find_direction_toward(enemy->cell_x, enemy->cell_y, target_x, target_y, &dx, &dy);
    return enemy_try_move_direction(enemy, world, -dx, -dy);
}

bool enemy_try_wander(GameObject *enemy, World *world) {

    int dx = (rand() % 3) - 1; // -1 0 | 1
    int dy = (rand() % 3) - 1; // -1 0 | 1

    if (dx == 0 && dy == 0) {
        dx = (rand() % 2) * 2 -1; // -1 | 1
    }

    update_entity_facing(enemy, dx, dy);

    return movement_try_mov(enemy, world, dx, dy);
}

void enemy_ai_update(GameObject *enemy, GameObject *player, World *world, float delta_time) {
    
    if (enemy->move_timer > 0.0f) {
        enemy->move_timer -= delta_time;
    }

    float dist_to_player = distance_between(
            enemy->cell_x, enemy->cell_y,
            player->cell_x, player->cell_y
    );

    if (enemy->alert_timer >  0.0f) {
        enemy->alert_timer -= delta_time;
    }

    switch (enemy->ai_state) {
        case AI_STATE_IDLE:
            if (dist_to_player < ALERT_DISTANCE) {
                enemy->ai_state = AI_STATE_ALERT;
                enemy->alert_timer = ALERT_DURATION;
            }
            break;
        case AI_STATE_ALERT:
            if (dist_to_player < ALERT_DISTANCE) {
                enemy->alert_timer = ALERT_DURATION;
                if (player->in_snake_form) {
                    enemy->ai_state = AI_STATE_FLEE;
                    enemy->move_speed = 1.2f;
                    enemy->animation_speed = 0.08f;
                } else {
                enemy->ai_state = AI_STATE_PURSUE;
                    enemy->move_speed = 1.1f;
                    enemy->animation_speed = 0.1f;
                }
            } else if (enemy->alert_timer <= 0.0f) {
                enemy->ai_state = AI_STATE_WANDER;
                enemy->wander_timer = 0.0f;
                enemy->move_speed = 0.6f;
                enemy->animation_speed = 0.18f;
            }
            break;
        case AI_STATE_PURSUE:
            if (dist_to_player < ALERT_DISTANCE) {
                if (enemy->move_timer <= 0.0f) {
                    if (enemy_try_move_toward(enemy, player->cell_x, player->cell_y, world)) {
                        enemy->move_timer = 0.2f;
                    }
                }

                if (player->in_snake_form) {
                    enemy->ai_state = AI_STATE_FLEE;
                }
            } else {
                enemy->ai_state = AI_STATE_ALERT;
                enemy->alert_timer = ALERT_DURATION;
            }
            break;
        case AI_STATE_FLEE:
            if (dist_to_player < ALERT_DISTANCE * 1.5f) {
                if (enemy->move_timer <= 0.0f) {
                    if (enemy_try_flee_from(enemy, player->cell_x, player->cell_y, world)) {
                        enemy->move_timer = 0.15f;
                    }
                }

                if (player->in_snake_form) {
                    enemy->ai_state = AI_STATE_PURSUE;
                }
            } else {
                enemy->ai_state = AI_STATE_ALERT;
                enemy->alert_timer = ALERT_DURATION;
            }
            break;
        case AI_STATE_WANDER:
            enemy->wander_timer -= delta_time;
                if (enemy->wander_timer <= 0.0f) {
                    enemy_try_wander(enemy, world);
                    enemy->wander_timer = WANDER_MOVE_INTERVAL;
                }

                if (dist_to_player < ALERT_DISTANCE) {
                    enemy->ai_state = AI_STATE_ALERT;
                    enemy->alert_timer = ALERT_DURATION;
                }
                break;
    }
}
