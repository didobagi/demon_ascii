#ifndef DIALOGUE_SYSTEM_H
#define DIALOGUE_SYSTEM_H

#include "types.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    COND_NONE = 0,
    COND_PLAYER_SNAKE_FORM = 1 << 0, 
    COND_PLAYER_DEMON_FORM = 1 << 1,
    COND_PLAYER_LOW_HEALTH = 1 << 2,
    COND_PLAYER_HIGH_HEALTH = 1 << 3,
    COND_ENEMY_FLEEING = 1 << 4,
    COND_ENEMY_PURUSE = 1 << 5,
    COND_ENEMY_LOW_HEALTH = 1 << 6,
    COND_FIRST_ENCOUNTER = 1 << 7,
} DialogueConditions;


typedef struct {
    DialogueOutcomeType type;
    DialogueDifficulty difficulty;
    int context_flags;
} DialogueOutcome;

typedef struct {
    char text[46];
    bool is_endpoint;
    int next_fragment_id;
    DialogueOutcome outcome;
} DialogueChoice;

typedef struct {
    int id;
    char text[512];
    uint32_t condition_flags;
    int choice_count;
    DialogueChoice choices[3];
} DialogueFragment;


typedef struct {
    DialogueFragment *fragments;
    int count;
    int capacity;
    char entity_name[32]; 
} DialogueFragmentPool;

typedef struct {
    DialogueFragmentPool *pools;
    int pool_count;
    int pool_capacity;
} DialogueLibrary;

DialogueLibrary* create_dialogue_library(void);
void destroy_dialogue_library(DialogueLibrary *library);

DialogueFragmentPool* load_dialogue_pool_from_file(const char *filepath, const char *entity_name);
bool add_pool_to_library(DialogueLibrary *library, DialogueFragmentPool *pool);

DialogueFragmentPool* get_dialogue_pool_for_entity(DialogueLibrary *library, EntityType entity_type);

#endif
