#include <stdlib.h>
#include <stdio.h>


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
            plot_pixel(x, y, 123);
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


    while (1) {

        
        // Wait for vertical sync and swap buffers
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
}