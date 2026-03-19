#include "player.h"
#include "image.h"

#define SCREEN_W  320
#define SCREEN_H  240

#define WINDOW_START_X 30
#define WINDOW_END_X   290
#define WINDOW_START_Y 30
#define WINDOW_END_Y   165

#define PLAYER_RADIUS 15
#define SHOOT_RADIUS  7

// Forward declarations for functions defined in one.c
void plot_pixel(int x, int y, short int line_color);
void safe_plot_pixel(int x, int y, short int colour);
void restore_background_area(int x_min, int x_max, int y_min, int y_max);


void update_player_position(struct player *p)
{
    // Save old position before updating
    p->old_x = p->x;
    p->old_y = p->y;

    p->x += p->dx;
    p->y += p->dy;

    // Clamp position to window bounds
    if (p->x < WINDOW_START_X) {    p->x = WINDOW_START_X;}
    else if (p->x > WINDOW_END_X) { p->x = WINDOW_END_X;}

    if (p->y < WINDOW_START_Y) {    p->y = WINDOW_START_Y;}
    else if (p->y > WINDOW_END_Y) { p->y = WINDOW_END_Y;}
}

void update_player_movement(struct player *p, int new_dx, int new_dy)
{
    p->dx = new_dx;
    p->dy = new_dy;
}

//Maybe change the shape to like a random splotter in the future, but for now just a solid circle
void shoot(struct player *p)
{
    for (int dy = -SHOOT_RADIUS; dy <= SHOOT_RADIUS; dy++) {
        for (int dx = -SHOOT_RADIUS; dx <= SHOOT_RADIUS; dx++) {

            if (dx * dx + dy * dy <= SHOOT_RADIUS * SHOOT_RADIUS) {
                int px = p->x + dx;
                int py = p->y + dy;

                // Check bounds (window bounds) and color both screen and background
                if (px >= WINDOW_START_X && px <= WINDOW_END_X && py >= WINDOW_START_Y && py <= WINDOW_END_Y) {
                    plot_pixel(px, py, p->colour);
                    background[py * SCREEN_W + px] = p->colour;
                }
            }
        }
    }
}

void draw_player(struct player *p)
{
    int size = PLAYER_RADIUS-5;
    int thickness = 2;
    short int colour = p->colour;

    // Draw vertical line (crosshair vertical bar)
    for (int i = -size; i <= size; i++) {
        for (int t = 0; t < thickness; t++) {
            safe_plot_pixel(p->x + t, p->y + i, 0);
        }
    }

    // Draw horizontal line (crosshair horizontal bar)
    for (int i = -size; i <= size; i++) {
        for (int t = 0; t < thickness; t++) {
            safe_plot_pixel(p->x + i, p->y + t, 0);
        }
    }

    // Draw circle using midpoint circle algorithm
    int x = 0;
    int y = PLAYER_RADIUS;
    int d = 3 - 2 * PLAYER_RADIUS;  // decision parameter

    while (x <= y) {
        // Draw 8 symmetric points with bounds checking
        for (int t = 0; t < thickness; t++) {
            safe_plot_pixel(p->x + x + t, p->y + y, colour);
            safe_plot_pixel(p->x - x + t, p->y + y, colour);
            safe_plot_pixel(p->x + x + t, p->y - y, colour);
            safe_plot_pixel(p->x - x + t, p->y - y, colour);
            safe_plot_pixel(p->x + y + t, p->y + x, colour);
            safe_plot_pixel(p->x - y + t, p->y + x, colour);
            safe_plot_pixel(p->x + y + t, p->y - x, colour);
            safe_plot_pixel(p->x - y + t, p->y - x, colour);
        }

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void restore_old_player_area(struct player *p)
{
    int size = PLAYER_RADIUS + 15 + 2;  // radius + crosshair + thickness
    int x_min = p->old_x - size;
    int x_max = p->old_x + size;
    int y_min = p->old_y - size;
    int y_max = p->old_y + size;

    restore_background_area(x_min, x_max, y_min, y_max);
}

void restore_background_area(int x_min, int x_max, int y_min, int y_max)
{
    // Clamp to window bounds
    if (x_min < WINDOW_START_X) {x_min = WINDOW_START_X;}
    if (x_max > WINDOW_END_X) {x_max = WINDOW_END_X;}
    if (y_min < WINDOW_START_Y) {y_min = WINDOW_START_Y;}
    if (y_max > WINDOW_END_Y) {y_max = WINDOW_END_Y;}
    
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            plot_pixel(x, y, background[y * SCREEN_W + x]);
        }
    }
}