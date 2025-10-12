#include "../include/map_template.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static TerrainType char_to_terrain (char c) {
    switch (c) {
        case '#': return TERRAIN_WALL;
        case 'W': return TERRAIN_OPTIONAL_WALL;
        case '.': return TERRAIN_FLOOR;       
        case '~': return TERRAIN_WATER;      
        case 'L': return TERRAIN_FIRE;
        default:
                  fprintf(stderr, "Warning: Unknown terrain character '%c', treating as floor\n", c);
                  return TERRAIN_FLOOR;
    }
}

static MarkerType char_to_marker(char c) {
    switch (c) {
        case '+': return MARKER_ENEMY_SPAWN;
        case '@': return MARKER_PLAYER_START;
        case '$': return MARKER_ITEM_SPAWN;
        case '^': return MARKER_EXIT;
        case '|': return MARKER_DOOR;
        default: return MARKER_NONE;
    }
}

//vibecoded load template fnc
Template* load_template_from_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open template file: %s\n", filepath);
        return NULL;
    }

    char name[64] = "unnamed";
    char line_buffer[512];
    if (fgets(line_buffer, sizeof(line_buffer), file)) {
        // Try to parse "NAME: something" format
        if (strncmp(line_buffer, "NAME:", 5) == 0) {
            // Skip past "NAME: " and any leading spaces
            char *name_start = line_buffer + 5;
            while (*name_start && isspace(*name_start)) name_start++;
            
            // Copy name, removing trailing newline
            strncpy(name, name_start, sizeof(name) - 1);
            name[sizeof(name) - 1] = '\0';
            char *newline = strchr(name, '\n');
            if (newline) *newline = '\0';
        } else {
            // First line wasn't a name line, so rewind to read it as template data
            rewind(file);
        }
    }
    
    // Now read the actual template lines
    // We'll store them temporarily to figure out dimensions
    char **lines = NULL;
    int line_count = 0;
    int line_capacity = 10;
    int max_line_length = 0;
    
    lines = malloc(sizeof(char*) * line_capacity);
    if (!lines) {
        fclose(file);
        return NULL;
    }
    
    // Read all lines
    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        // Skip empty lines
        int len = strlen(line_buffer);
        if (len == 0 || line_buffer[0] == '\n') continue;
        
        // Remove trailing newline
        if (line_buffer[len - 1] == '\n') {
            line_buffer[len - 1] = '\0';
            len--;
        }
        
        // Skip if line is now empty after removing newline
        if (len == 0) continue;
        
        // Track maximum line length
        if (len > max_line_length) max_line_length = len;
        
        // Grow lines array if needed
        if (line_count >= line_capacity) {
            line_capacity *= 2;
            char **new_lines = realloc(lines, sizeof(char*) * line_capacity);
            if (!new_lines) {
                // Cleanup on allocation failure
                for (int i = 0; i < line_count; i++) free(lines[i]);
                free(lines);
                fclose(file);
                return NULL;
            }
            lines = new_lines;
        }
        
        // Store a copy of this line
        lines[line_count] = strdup(line_buffer);
        if (!lines[line_count]) {
            // Cleanup on allocation failure
            for (int i = 0; i < line_count; i++) free(lines[i]);
            free(lines);
            fclose(file);
            return NULL;
        }
        line_count++;
    }
    
    fclose(file);
    
    // Check if we got any template data
    if (line_count == 0 || max_line_length == 0) {
        fprintf(stderr, "Template file %s contains no valid template data\n", filepath);
        free(lines);
        return NULL;
    }
    
    // Now we know dimensions: width = max_line_length, height = line_count
    int width = max_line_length;
    int height = line_count;
    
    // Allocate the template structure
    Template *template = malloc(sizeof(Template));
    if (!template) {
        for (int i = 0; i < line_count; i++) free(lines[i]);
        free(lines);
        return NULL;
    }
    
    // Copy name and dimensions
    strncpy(template->name, name, sizeof(template->name) - 1);
    template->name[sizeof(template->name) - 1] = '\0';
    template->width = width;
    template->height = height;
    
    // Allocate terrain and marker arrays
    int total_cells = width * height;
    template->terrain = malloc(sizeof(TerrainType) * total_cells);
    template->markers = malloc(sizeof(MarkerType) * total_cells);
    
    if (!template->terrain || !template->markers) {
        if (template->terrain) free(template->terrain);
        if (template->markers) free(template->markers);
        free(template);
        for (int i = 0; i < line_count; i++) free(lines[i]);
        free(lines);
        return NULL;
    }
    
    // Initialize all cells to floor with no markers
    for (int i = 0; i < total_cells; i++) {
        template->terrain[i] = TERRAIN_FLOOR;
        template->markers[i] = MARKER_NONE;
    }
    
    // Parse the lines into terrain and markers
    for (int y = 0; y < line_count; y++) {
        int line_len = strlen(lines[y]);
        for (int x = 0; x < line_len; x++) {
            char c = lines[y][x];
            int index = y * width + x;
            
            // Check if this character is a marker
            MarkerType marker = char_to_marker(c);
            if (marker != MARKER_NONE) {
                // It's a marker - set marker and make terrain floor
                template->markers[index] = marker;
                template->terrain[index] = TERRAIN_FLOOR;
            } else {
                // It's terrain
                template->terrain[index] = char_to_terrain(c);
            }
        }
    }
    
    // Free the temporary line storage
    for (int i = 0; i < line_count; i++) free(lines[i]);
    free(lines);
    
    printf("Loaded template '%s': %dx%d\n", template->name, width, height);
    return template;
}

void destroy_template(Template *templ)  {
    if (!templ) return;
    if (templ->terrain) free(templ->terrain);
    if (templ->markers) free(templ->markers);
    free(templ);
}

TemplateLibrary* create_template_library (void) {
    TemplateLibrary *library = malloc(sizeof(TemplateLibrary));
    if (!library) return NULL;

    library->capacity = 20;
    library->count = 0;
    library->templates = malloc(sizeof(Template) * library->capacity);

    if (!library->templates) {
        free(library);
        return NULL;
    }

    return library;
}

void destroy_template_library(TemplateLibrary *library) {
    if (!library) return;

    for (int i = 0;i < library->count;i ++) {
        if (library->templates[i].terrain) free(library->templates[i].terrain);
        if (library->templates[i].markers) free(library->templates[i].markers);
    }

    free(library->templates);
    free(library);

}

bool add_template_to_library(TemplateLibrary *library, Template *templ) {
    if (!library || !templ) return false;
    if (library->count >= library->capacity) {
        int new_capacity = library->capacity * 2;
        Template *new_templates = realloc(library->templates, 
                sizeof(Template) * new_capacity);
        if (!new_templates) return false;

        library->templates = new_templates;
        library->capacity = new_capacity;
    }

    library->templates[library->count] = *templ;
    library->count++;

    free(templ);

    return true;
}

Template* get_template_by_name(TemplateLibrary *library, const char *name) {
    if (!library || !name) return NULL;

    for (int i = 0; i < library->count; i++) {
        if (strcmp(library->templates[i].name, name) == 0) {
            return &library->templates[i];
        }
    }
    return NULL;
}

Template* get_random_template(TemplateLibrary *library) {
    if (!library || library->count == 0) return NULL;

    int index = rand() % library->count;
    return &library->templates[index];
}
