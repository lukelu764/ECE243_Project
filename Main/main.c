#include <stdlib.h>
#include <stdio.h>
#include "image.h"
#include "player.h"

volatile int pixel_buffer_start;

// Forward declarations
void plot_pixel(int x, int y, short int line_color);

#define SCREEN_W  320
#define SCREEN_H  240
#define KEY_BASE 0xFF200050
#define SW_BASE	 0xFF200040

// Window boundaries
#define WINDOW_START_X 30
#define WINDOW_END_X   290
#define WINDOW_START_Y 30
#define WINDOW_END_Y   165

void read_keyboard_input(struct player *p)
{
    volatile int *key_ptr = (int *)KEY_BASE;
    int key_value = *key_ptr;
    
    // Check which key is pressed (bits 0-3 represent keys 0-3)
    if (key_value & 0x1) {           // Key 0 pressed - move right
        update_player_movement(p, 1, 0);
    } else if (key_value & 0x2) {    // Key 1 pressed - move left
        update_player_movement(p, -1, 0);
    } else if (key_value & 0x4) {    // Key 2 pressed - move down
        update_player_movement(p, 0, 1);
    } else if (key_value & 0x8) {    // Key 3 pressed - move up
        update_player_movement(p, 0, -1);
    } else {                         // No key pressed - stop
        update_player_movement(p, 0, 0);
    }
}
void read_switch_input(struct player *p)
{
    volatile int *sw_ptr = (int *)SW_BASE;
    int sw_value = *sw_ptr;
    
    // Check if switch 0 is pressed (bit 0)
    if (sw_value & 0x1) {
        shoot(p);
    }
}

void safe_plot_pixel(int x, int y, short int colour)
{
    // Check bounds within the window
    if (x >= WINDOW_START_X && x <= WINDOW_END_X && y >= WINDOW_START_Y && y <= WINDOW_END_Y) {
        plot_pixel(x, y, colour);
    }
}


void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;
    one_pixel_address = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *one_pixel_address = line_color;
}

void wait_for_vsync(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int *status_ptr     = pixel_ctrl_ptr + 3;

    *pixel_ctrl_ptr = 1; // write 1 to Buffer register (offset +0) to trigger swap

    while ((*status_ptr & 0x1) != 0)
        ;
}

void draw_background(void)
{
    for (int y = 0; y < SCREEN_H; y++) {
        for (int x = 0; x < SCREEN_W; x++) {
            plot_pixel(x, y, background[y * SCREEN_W + x]);
        }
    }
}

int main(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    short int Buffer1[240][512];
    short int Buffer2[240][512];


    /* set front pixel buffer to Buffer1 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; 
    wait_for_vsync();


    pixel_buffer_start = *pixel_ctrl_ptr;
    draw_background();

    *(pixel_ctrl_ptr + 1) = (int) &Buffer2; 
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    draw_background();

    struct player p1 = {160, 120, 160, 120, 1, 0, 0xF800}; 
    struct player p2 = {100, 100, 100, 100, 0, 1, 0x001F};

    while (1) {
        // Read keyboard input for player 1
        read_keyboard_input(&p1);
        
        // Read switch input for player 1 to shoot
        read_switch_input(&p1);
        
        // Update positions (old positions are saved inside update_player_position)
        update_player_position(&p1);
        update_player_position(&p2);

        // Restore background where players WERE (old positions)
        restore_old_player_area(&p1);
        restore_old_player_area(&p2);

        // Draw players at NEW positions
        draw_player(&p1);
        draw_player(&p2);
        
        // Wait for vertical sync and swap buffers
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
}
