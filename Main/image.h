#ifndef BACKGROUND_DATA_H
#define BACKGROUND_DATA_H

#ifndef SCREEN_W
    #define SCREEN_W  320
#endif

#ifndef SCREEN_H
    #define SCREEN_H  240
#endif

// 'extern' tells the compiler the array is defined in another file
extern unsigned short background[SCREEN_H * SCREEN_W];

#endif