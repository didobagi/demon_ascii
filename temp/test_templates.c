#include <stdio.h>
#include <stdlib.h>
#include "../include/map_template.h"

// Print a template to the console for visual verification
void print_template(Template *template) {
    printf("\nTemplate: %s\n", template->name);
    printf("Dimensions: %dx%d\n", template->width, template->height);
    printf("Terrain:\n");
    
    for (int y = 0; y < template->height; y++) {
        for (int x = 0; x < template->width; x++) {
            int index = y * template->width + x;
            TerrainType terrain = template->terrain[index];
            MarkerType marker = template->markers[index];
            
            // If there's a marker, show it instead of terrain
            if (marker != MARKER_NONE) {
                switch (marker) {
                    case MARKER_PLAYER_START: printf("@"); break;
                    case MARKER_ENEMY_SPAWN: printf("+"); break;
                    case MARKER_ITEM_SPAWN: printf("$"); break;
                    case MARKER_EXIT: printf("^"); break;
                    case MARKER_DOOR: printf("|"); break;
                    default: printf("?"); break;
                }
            } else {
                // Show terrain
                switch (terrain) {
                    case TERRAIN_WALL: printf("#"); break;
                    case TERRAIN_OPTIONAL_WALL: printf("W"); break;
                    case TERRAIN_FLOOR: printf("."); break;
                    default: printf("?"); break;
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <template_file>\n", argv[0]);
        return 1;
    }
    
    // Load the template
    printf("Loading template from: %s\n", argv[1]);
    Template *template = load_template_from_file(argv[1]);
    
    if (!template) {
        printf("Failed to load template\n");
        return 1;
    }
    
    // Print it for verification
    print_template(template);
    
    // Cleanup
    destroy_template(template);
    
    return 0;
}
