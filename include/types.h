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

typedef enum {
    AI_STATE_IDLE,
    AI_STATE_ALERT,
    AI_STATE_PURSUE,
    AI_STATE_FLEE,
    AI_STATE_WANDER,
    //AI_STATE_DEAD,
} AIState;

typedef enum {
    TERRAIN_FLOOR,
    TERRAIN_WALL,
    TERRAIN_OPTIONAL_WALL,
    TERRAIN_FIRE,
    TERRAIN_WATER,
    TERRAIN_GRASS,
} TerrainType;

typedef enum {
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_COLLECTIBLE,
} EntityType;

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

typedef struct Shape {
    Point *original_points;
    Point *rotated_points;
    float *distances;
    int point_count;
    char display_char;
    TextureType texture;
} Shape;

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

typedef struct CollisionData{
    bool in_world;
} CollisionData;

typedef struct GameObject {
    EntityType entity_type;
    bool active;
    int cell_x, cell_y;
    float v_x, v_y;
    float target_v_x, target_v_y;
    bool is_moving;
    float move_speed;
    float move_start_x, move_start_y;
    float move_time;

    Transform transform;
    Shape shape;
    ShapeBounds bounds;
    Color color;
    CollisionData collision;

    AIState current_state;
    unsigned int state_entered_frame;
    float speed;

    Point *demon_form_template;
    int demon_form_point_count;
    Point *snake_form_template;
    int snake_form_point_count;
    bool is_morphing;
    bool in_snake_form;
    bool point_collected[100];
    int total_collected_count;

    //int  health;
    //int  max_health;
    //unsigned int last_damage_frame;
} GameObject;

typedef struct {
    TerrainType terrain;
    GameObject **entities;  //din array of entity pointers
    int entity_count;
    int entity_capacity;

} Cell;

typedef struct {
    int width;
    int height;
    Cell *grid;
} World;

typedef struct {
    float x_float, y_float;
    int x;
    int y;
    int width;
    int height;
} Camera;

typedef struct Enemy{
    float x, y;
    float dx, dy;
    Point *shape_template;
    int point_count;
    Point rotated_points[100];
    float angle;
    ShapeBounds bounds;
    float speed;
    bool active;

    AIState current_state;
    unsigned int state_entered_frame;
    int health;
    int max_health;
} Enemy;

typedef struct GameState {
    World *world;
    GameObject objects[MAX_OBJECTS];
    int object_count;
    Sector sectors[SECTOR_ROWS][SECTOR_COLS];
    int max_x;
    int max_y;
    unsigned int frame;

    CollectiblePoint collectibles[50];
    int collectible_count;

    int enemy_count;
} GameState;

typedef struct KeyState {
    bool up;
    bool down;
    bool left;
    bool right;
    bool quit;
} KeyState;

#endif
