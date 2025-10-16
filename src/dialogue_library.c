#include "../include/dialogue_library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static uint32_t parse_conditions(const char *cond_str) {
    uint32_t flags = COND_NONE;
    
    if (strstr(cond_str, "SNAKE_FORM")) flags |= COND_PLAYER_SNAKE_FORM;
    if (strstr(cond_str, "DEMON_FORM")) flags |= COND_PLAYER_DEMON_FORM;
    if (strstr(cond_str, "LOW_HEALTH")) flags |= COND_PLAYER_LOW_HEALTH;
    if (strstr(cond_str, "HIGH_HEALTH")) flags |= COND_PLAYER_HIGH_HEALTH;
    if (strstr(cond_str, "ENEMY_FLEEING")) flags |= COND_ENEMY_FLEEING;
    if (strstr(cond_str, "ENEMY_AGGRESSIVE")) flags |= COND_ENEMY_PURUSE;
    if (strstr(cond_str, "FIRST_ENCOUNTER")) flags |= COND_FIRST_ENCOUNTER;
    if (strstr(cond_str, "NONE")) flags = COND_NONE;
    
    return flags;
}

static bool parse_choice(const char *line, DialogueChoice *choice) {
    // Format: "CHOICE: text | TYPE | VALUE"
    char *choice_start = strchr(line, ':');
    if (!choice_start) return false;
    choice_start++; // Skip ':'
    
    // Skip whitespace
    while (*choice_start && isspace(*choice_start)) choice_start++;
    
    // Parse choice text (everything before first |)
    char *pipe1 = strchr(choice_start, '|');
    if (!pipe1) return false;
    
    int text_len = pipe1 - choice_start;
    if (text_len >= 64) text_len = 63;
    strncpy(choice->text, choice_start, text_len);
    choice->text[text_len] = '\0';
    
    // Trim trailing whitespace from text
    for (int i = text_len - 1; i >= 0 && isspace(choice->text[i]); i--) {
        choice->text[i] = '\0';
    }
    
    // Parse outcome type
    char *type_start = pipe1 + 1;
    while (*type_start && isspace(*type_start)) type_start++;
    
    char *pipe2 = strchr(type_start, '|');
    if (!pipe2) return false;
    
    char type_str[32];
    int type_len = pipe2 - type_start;
    if (type_len >= 32) type_len = 31;
    strncpy(type_str, type_start, type_len);
    type_str[type_len] = '\0';
    
    // Trim whitespace
    for (int i = type_len - 1; i >= 0 && isspace(type_str[i]); i--) {
        type_str[i] = '\0';
    }
    
    // Parse value
    char *value_start = pipe2 + 1;
    while (*value_start && isspace(*value_start)) value_start++;
    
    // Determine outcome type
    if (strcmp(type_str, "COMBAT") == 0) {
        choice->is_endpoint = true;
        choice->next_fragment_id = -1;
        choice->outcome.type = OUTCOME_END_COMBAT;
    } else if (strcmp(type_str, "PUZZLE") == 0) {
        choice->is_endpoint = true;
        choice->next_fragment_id = -1;
        choice->outcome.type = OUTCOME_END_PUZZLE;
    } else if (strcmp(type_str, "DEXTERITY") == 0) {
        choice->is_endpoint = true;
        choice->next_fragment_id = -1;
        choice->outcome.type = OUTCOME_END_DEXTERITY;
    } else if (strcmp(type_str, "PEACEFUL") == 0) {
        choice->is_endpoint = true;
        choice->next_fragment_id = -1;
        choice->outcome.type = OUTCOME_END_PEACEFUL;
    } else if (strcmp(type_str, "FRAGMENT") == 0) {
        choice->is_endpoint = false;
        choice->next_fragment_id = atoi(value_start);
        choice->outcome.type = OUTCOME_CONTINUE;
    } else {
        return false;
    }
    
    // Parse difficulty if endpoint
    if (choice->is_endpoint) {
        if (strstr(value_start, "EASY")) {
            choice->outcome.difficulty = DIFFICULTY_EASY;
        } else if (strstr(value_start, "MEDIUM")) {
            choice->outcome.difficulty = DIFFICULTY_MEDIUM;
        } else if (strstr(value_start, "HARD")) {
            choice->outcome.difficulty = DIFFICULTY_HARD;
        } else {
            choice->outcome.difficulty = DIFFICULTY_MEDIUM;
        }
    }
    
    choice->outcome.context_flags = 0;
    
    return true;
}

DialogueFragmentPool* load_dialogue_pool_from_file(const char *filepath, const char *entity_name) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open dialogue file: %s\n", filepath);
        return NULL;
    }
    
    DialogueFragmentPool *pool = malloc(sizeof(DialogueFragmentPool));
    if (!pool) {
        fclose(file);
        return NULL;
    }
    
    pool->capacity = 10;
    pool->count = 0;
    pool->fragments = malloc(sizeof(DialogueFragment) * pool->capacity);
    strncpy(pool->entity_name, entity_name, sizeof(pool->entity_name) - 1);
    pool->entity_name[sizeof(pool->entity_name) - 1] = '\0';
    
    if (!pool->fragments) {
        free(pool);
        fclose(file);
        return NULL;
    }
    
    char line[1024];
    DialogueFragment current_fragment;
    bool reading_fragment = false;
    int current_choice_index = 0;
    
    memset(&current_fragment, 0, sizeof(DialogueFragment));
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        // Check for fragment separator
        if (strncmp(line, "---", 3) == 0) {
            if (reading_fragment) {
                // Save current fragment
                current_fragment.choice_count = current_choice_index;
                
                // Grow array if needed
                if (pool->count >= pool->capacity) {
                    pool->capacity *= 2;
                    DialogueFragment *new_frags = realloc(pool->fragments,
                                                          sizeof(DialogueFragment) * pool->capacity);
                    if (!new_frags) {
                        free(pool->fragments);
                        free(pool);
                        fclose(file);
                        return NULL;
                    }
                    pool->fragments = new_frags;
                }
                
                pool->fragments[pool->count] = current_fragment;
                pool->count++;
                
                // Reset for next fragment
                memset(&current_fragment, 0, sizeof(DialogueFragment));
                reading_fragment = false;
                current_choice_index = 0;
            }
            continue;
        }
        
        // Parse fragment fields
        if (strncmp(line, "ID:", 3) == 0) {
            current_fragment.id = atoi(line + 3);
            reading_fragment = true;
        } else if (strncmp(line, "CONDITIONS:", 11) == 0) {
            current_fragment.condition_flags = parse_conditions(line + 11);
        } else if (strncmp(line, "TEXT:", 5) == 0) {
            char *text_start = line + 5;
            while (*text_start && isspace(*text_start)) text_start++;
            strncpy(current_fragment.text, text_start, sizeof(current_fragment.text) - 1);
            current_fragment.text[sizeof(current_fragment.text) - 1] = '\0';
        } else if (strncmp(line, "CHOICE:", 7) == 0) {
            if (current_choice_index < 3) {
                if (parse_choice(line, &current_fragment.choices[current_choice_index])) {
                    current_choice_index++;
                }
            }
        }
    }
    
    // Save last fragment if file doesn't end with ---
    if (reading_fragment) {
        current_fragment.choice_count = current_choice_index;
        if (pool->count < pool->capacity) {
            pool->fragments[pool->count] = current_fragment;
            pool->count++;
        }
    }
    
    fclose(file);
    
    printf("Loaded %d dialogue fragments from %s for %s\n", 
           pool->count, filepath, entity_name);
    
    return pool;
}

DialogueLibrary* create_dialogue_library(void) {
    DialogueLibrary *library = malloc(sizeof(DialogueLibrary));
    if (!library) return NULL;
    
    library->pool_capacity = 10;
    library->pool_count = 0;
    library->pools = malloc(sizeof(DialogueFragmentPool) * library->pool_capacity);
    
    if (!library->pools) {
        free(library);
        return NULL;
    }
    
    return library;
}

void destroy_dialogue_library(DialogueLibrary *library) {
    if (!library) return;
    
    for (int i = 0; i < library->pool_count; i++) {
        if (library->pools[i].fragments) {
            free(library->pools[i].fragments);
        }
    }
    
    free(library->pools);
    free(library);
}

bool add_pool_to_library(DialogueLibrary *library, DialogueFragmentPool *pool) {
    if (!library || !pool) return false;
    
    if (library->pool_count >= library->pool_capacity) {
        library->pool_capacity *= 2;
        DialogueFragmentPool *new_pools = realloc(library->pools,
                                                   sizeof(DialogueFragmentPool) * library->pool_capacity);
        if (!new_pools) return false;
        library->pools = new_pools;
    }
    
    library->pools[library->pool_count] = *pool;
    library->pool_count++;
    
    free(pool);  // Pool data copied, free the wrapper
    
    return true;
}

DialogueFragmentPool* get_dialogue_pool_for_entity(DialogueLibrary *library, EntityType entity_type) {
    if (!library) return NULL;
    
    // For now, just return first pool (we'll map entity types to names later)
    // TODO: Add mapping from EntityType to entity name
    if (library->pool_count > 0) {
        return &library->pools[0];
    }
    
    return NULL;
}
