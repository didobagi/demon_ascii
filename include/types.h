#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define MAX_OBJECTS 50
#define SECTOR_COLS 8
#define SECTOR_ROWS 4
#define CORE_SNAKE_POINTS 15

typedef struct Sector {
    bool is_dangerous;
    unsigned int state_change_frame;
}Sector;

typedef enum {
    COLOR_BLACK = 0,
    COLOR_RED = 1,
    COLOR_GREEN = 2,
    COLOR_YELLOW = 3,
    COLOR_BLUE = 4,
    COLOR_MAGENTA = 5,
    COLOR_CYAN = 6,
    COLOR_WHITE = 7,
    COLOR_BRIGHT_BLACK = 8,   
    COLOR_BRIGHT_RED = 9,
    COLOR_BRIGHT_GREEN = 10,
    COLOR_BRIGHT_YELLOW = 11,
    COLOR_BRIGHT_BLUE = 12,
    COLOR_BRIGHT_MAGENTA = 13,
    COLOR_BRIGHT_CYAN = 14,
    COLOR_BRIGHT_WHITE = 15
} Color;

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

typedef struct CollectiblePoint {
    float x, y;
    int shape_point_index;
    bool active;
    //float drift angle;
    //float drift speed;
} CollectiblePoint;


typedef struct GameObject {
    Transform transform;
    VisualShape shape;
    CollisionShape collision;
    ShapeBounds bounds;
    bool active;
    Color color;

    Point *demon_form_template;
    int demon_form_point_count;
    Point *snake_form_template;
    int snake_form_point_count;
    bool is_morphing;
    bool in_snake_form;

    bool point_collected[100];
    int total_collected_count;

} GameObject;

typedef struct GameState {
    GameObject objects[MAX_OBJECTS];
    int object_count;
    Sector sectors[SECTOR_ROWS][SECTOR_COLS];
    int max_x;
    int max_y;
    unsigned int frame;

    CollectiblePoint collectibles[50];
    int collectible_count;
} GameState;

typedef struct KeyState {
    bool up;
    bool down;
    bool left;
    bool right;
    bool quit;
} KeyState;

#endif
