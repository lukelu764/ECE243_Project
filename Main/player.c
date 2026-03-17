volatile int pixel_buffer_start;

#define SCREEN_W  320
#define SCREEN_H  240
#define KEY_BASE 0xFF200050
#define SW_BASE	 0xFF200040

// Single array for background storage (replaces both colour_data and background)
short int background[SCREEN_H][SCREEN_W];

struct player
{
    int x;
    int y;
    int old_x;
    int old_y;

    int dx;
    int dy;
    int colour;
};

void update_player_position(struct player *p)
{
    // Save old position before updating
    p->old_x = p->x;
    p->old_y = p->y;
    
    p->x += p->dx;
    p->y += p->dy;

    if (p->x < 0) {
        p->x = 0;
    }else if (p->x >= SCREEN_W) {
        p->x = SCREEN_W - 1;
    }

    if (p->y < 0) {
        p->y = 0;
    }else if (p->y >= SCREEN_H) {
        p->y = SCREEN_H - 1;
    }
}

void update_player_movement(struct player *p, int new_dx, int new_dy)
{
    p->dx = new_dx;
    p->dy = new_dy;
}

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

void shoot(struct player *p, int radius)
{
    // Fill a solid circle around the player with their color
    // and save it to the background array for permanent change
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            // Check if point is within circle using distance formula
            if (dx * dx + dy * dy <= radius * radius) {
                int px = p->x + dx;
                int py = p->y + dy;
                
                // Check bounds and color both screen and background
                if (px >= 0 && px < SCREEN_W && py >= 0 && py < SCREEN_H) {
                    plot_pixel(px, py, p->colour);
                    background[py][px] = p->colour;
                }
            }
        }
    }
}

void read_switch_input(struct player *p)
{
    volatile int *sw_ptr = (int *)SW_BASE;
    int sw_value = *sw_ptr;
    
    // Check if switch 0 is pressed (bit 0)
    if (sw_value & 0x1) {
        shoot(p, 15);  // Shoot with a radius of 15 pixels (smaller than crosshair radius of 25)
    }
}

void safe_plot_pixel(int x, int y, short int colour)
{
    // Check bounds before plotting
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        plot_pixel(x, y, colour);
    }
}

void draw_player(struct player *p)
{
    int size = 15;
    int thickness = 2;  
    short int colour = p->colour;
    int radius = 25; 
    
    // Draw vertical line (crosshair vertical bar)
    for (int i = -size; i <= size; i++) {
        for (int t = 0; t < thickness; t++) {
            safe_plot_pixel(p->x + t, p->y + i, colour);
        }
    }
    
    // Draw horizontal line (crosshair horizontal bar)
    for (int i = -size; i <= size; i++) {
        for (int t = 0; t < thickness; t++) {
            safe_plot_pixel(p->x + i, p->y + t, colour);
        }
    }
    
    // Draw circle using midpoint circle algorithm
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;  // decision parameter
    
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

void clear_screen(void)
{
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            plot_pixel(x, y, 0x0000);
}

void init_background(short int default_color)
{
    // Initialize background with default color
    for (int y = 0; y < SCREEN_H; y++) {
        for (int x = 0; x < SCREEN_W; x++) {
            background[y][x] = default_color;
        }
    }
}

void restore_background_area(int x_min, int x_max, int y_min, int y_max)
{
    // Clamp to screen bounds
    if (x_min < 0) x_min = 0;
    if (x_max >= SCREEN_W) x_max = SCREEN_W - 1;
    if (y_min < 0) y_min = 0;
    if (y_max >= SCREEN_H) y_max = SCREEN_H - 1;
    
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            plot_pixel(x, y, background[y][x]);
        }
    }
}

void restore_old_player_area(struct player *p)
{
    int size = 25 + 15 + 2;  // radius + crosshair + thickness
    int x_min = p->old_x - size;
    int x_max = p->old_x + size;
    int y_min = p->old_y - size;
    int y_max = p->old_y + size;
    
    restore_background_area(x_min, x_max, y_min, y_max);
}

void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;

    one_pixel_address = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));

    *one_pixel_address = line_color;
}

//temp code for testing-----------------------------------
short int Buffer1[240][512];
short int Buffer2[240][512];

void wait_for_vsync(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int *status_ptr     = pixel_ctrl_ptr + 3;

    *pixel_ctrl_ptr = 1; // write 1 to Buffer register (offset +0) to trigger swap

    while ((*status_ptr & 0x1) != 0)
        ;
}

// Draw background from the background array to the screen
void draw_background(void)
{
    for (int y = 0; y < SCREEN_H; y++) {
        for (int x = 0; x < SCREEN_W; x++) {
            plot_pixel(x, y, background[y][x]);
        }
    }
}

int main(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;

    short int current_frame[SCREEN_H][SCREEN_W];
    short int next_frame[SCREEN_H][SCREEN_W];
    // declare other variables(not shown)

    /* set front pixel buffer to Buffer1 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; 
    wait_for_vsync();

    // Initialize background with green color (0x07E0)
    init_background(0x07E0);

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
        
        // Read switch input for player 1 to shoot with radius 15 (smaller than crosshair radius of 25)
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