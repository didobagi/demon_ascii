#ifndef MAP_TEMPLATE_H
#define MAP_TEMPLATE_H

#include "types.h"
#include <stdbool.h>

typedef enum {
    MARKER_NONE,
    MARKER_PLAYER_START,
    MARKER_ENEMY_SPAWN,
    MARKER_ITEM_SPAWN,
    MARKER_EXIT,
    MARKER_DOOR,
} MarkerType;

typedef struct {
    char name[64];
    int width;
    int height;
    TerrainType *terrain;
    MarkerType *markers;
} Template;

typedef struct {
    Template *templates;
    int count;
    int capacity;
} TemplateLibrary;

Template* load_template_from_file (const char *filepath);
void destroy_template (Template *templ);

TemplateLibrary* create_template_library (void);
void destroy_template_library (TemplateLibrary *library);
bool add_template_to_library (TemplateLibrary *library, Template *templ);
Template* get_template_by_name (TemplateLibrary *library, const char *name);
Template* get_random_template (TemplateLibrary *library);

#endif
