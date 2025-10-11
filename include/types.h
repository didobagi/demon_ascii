#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define MAX_OBJECTS 50
#define SECTOR_COLS 8
#define SECTOR_ROWS 4

typedef struct Sector {
    bool is_dangerous;
    unsigned int state_change_frame;
}Sector;


typedef enum {
    TEXTURE_SOLID,
    TEXTURE_GRADIENT,
    TEXTURE_FIRE,
    TEXTURE_SPEED,
} TextureType;

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Transform {
    float x, y;
    float new_x, new_y;
    float angle;
    float dx, dy;
} Transform;

typedef struct VisualShape {
    Point *original_points;
    Point *rotated_points;
    float *distances;
    int point_count;
    char display_char;
    TextureType texture;
} VisualShape;

typedef struct ShapeBounds {
    float min_x, max_x;
    float min_y, max_y;
} ShapeBounds;


typedef struct CollisionCircle {
    float x, y;
    float radius;
} CollisionCircle;

typedef struct CollisionShape {
    CollisionCircle *circles;
    int circle_count;
    float max_radius;
} CollisionShape;

typedef struct GameObject {
    Transform transform;
    VisualShape shape;
    CollisionShape collision;
    ShapeBounds bounds;
    bool active;
    Point *demon_form_template;
    int demon_form_point_count;
    Point *snake_form_template;
    int snake_form_point_count;
    bool is_morphing;
    bool in_snake_form;
} GameObject;

typedef struct GameState {
    GameObject objects[MAX_OBJECTS];
    int object_count;
    Sector sectors[SECTOR_ROWS][SECTOR_COLS];
    int max_x;
    int max_y;
    unsigned int frame;
} GameState;

typedef struct KeyState {
    bool up;
    bool down;
    bool left;
    bool right;
    bool quit;
} KeyState;

#endif
