#ifndef DIALOGUE_LIBRARY_H
#define DIALOGUE_LIBRARY_H

#include "dialogue.h"
#include "types.h"

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
