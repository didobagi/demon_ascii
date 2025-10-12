#include "../include/splash.h"
#include "../include/render.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>


static void wait_for_kp (void) {
    struct termios original, raw;
    tcgetattr(STDIN_FILENO, &original);
    raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;  
    raw.c_cc[VTIME] = 0;  
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    char c;
    read(STDIN_FILENO, &c, 1); 
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

static void animate_text_morph (const char *text, int start_x, int start_y) {
    int text_len = 0;
    for(const char *p = text; *p != '\0'; p++) text_len++;
    
    int morph_timers[100];
    for (int i = 0; i < text_len; i++) {
        morph_timers[i] = 10 + (rand() % 31);
    }

    const char morph_chars[] = {'#', '@', '*', '%', '+', '=', '-', '~', 'O', '0', 'X', 'A'};
    int morph_chars_count = 12;

    bool all_done = false;

    while (!all_done) {
        all_done = true;

        for (int i = 0; i < text_len; i++) {
            if (morph_timers[i] > 1) {
                all_done = false;
                char random_char = morph_chars[rand() % morph_chars_count];
                printf("\033[%d;%dH%c", start_y, start_x + i, random_char);
                morph_timers[i]--;
            } else if (morph_timers[i] == 1) {
                printf("\033[%d;%dH\033[90mA\033[0m", start_y, start_x + i);
                morph_timers[i]--;
            } else {
                printf("\033[%d;%dH\033[90mA\033[0m", start_y, start_x + i);
            }
        }
        fflush(stdout);
        usleep(50000);
    }

    usleep(500000);
}

static void wait_for_keypress(void) {

    struct termios original, raw;
    tcgetattr(STDIN_FILENO, &original);
    
    raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;   
    raw.c_cc[VTIME] = 0; 
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    char c;
    read(STDIN_FILENO, &c, 1);  // Blocks here
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void show_splash_screen(int term_width, int term_height) {
    printf("\033[2J");      
    printf("\033[?25l");   
    fflush(stdout);
    
    const char *message = "PRESS ANY KEY TO START";
    
    render_text_centered(message, term_width, term_height, -2);
    
    wait_for_keypress();
    
    int text_len = 0;
    for (const char *p = message; *p != '\0'; p++) text_len++;
    int center_x = term_width / 2;
    int center_y = term_height / 2;
    int text_x = center_x - (text_len / 2);
    int text_y = center_y - 2;
    
    animate_text_morph(message, text_x, text_y);
}
