#ifndef PLAYER_H
#define PLAYER_H

struct player {
    int x, y;
    int old_x, old_y;
    int dx, dy;
    int colour;
};

void update_player_position(struct player *p);
void update_player_movement(struct player *p, int new_dx, int new_dy);
void shoot(struct player *p);
void draw_player(struct player *p);
void restore_old_player_area(struct player *p);

#endif