#include "blob.h"
#include "mbed.h"

extern Serial pc;


// Randomly initialize a blob's position, velocity, color, radius, etc.
// Set the valid flag to true and the delete_now flag to false.
// delete_now is basically the derivative of valid. It goes true for one
// fram when the blob is deleted, and then it is reset to false in the next frame
// when that blob is deleted.
void BLOB_init(BLOB* b) {
    b->posx = rand() % (WORLD_WIDTH + 1);
    b->old_x = (*b).posx;
    b->posy = rand() % (WORLD_HEIGHT + 1);
    b->old_y = (*b).posy;
    b->rad = 5;
    // Fix velocity for food blob cases
    float th = rand() % 361;
    b->vx = (MAX_VEL * cos(th * PI / 180)) / (*b).rad;
    b->vy = (MAX_VEL * sin(th * PI / 180)) / (*b).rad; 
    b->color = FOOD_COL; // make way to generate random color value
    b->valid = true;
    b->delete_now = false;
}


// Take in a blob and determine whether it is inside the world.
// If the blob has escaped the world, put it back on the edge
// of the world and negate its velocity so that it bounces off
// the boundary. Use WORLD_WIDTH and WORLD_HEIGHT defined in "misc.h"
void BLOB_constrain2world(BLOB* b) {
    if(((*b).posx - (*b).rad) < 0) {
        b->posx = (*b).rad;
        b->vx = -(*b).vx;
    } else if (((*b).posx + (*b).rad) > WORLD_WIDTH) {
        b->posx = WORLD_WIDTH - (*b).rad;
        b->vx = -(*b).vx;
    } else if(((*b).posy - (*b).rad) < 0) {
        b->posy = (*b).rad;
        b->vy = -(*b).vy;
    } else if (((*b).posy + (*b).rad) > WORLD_HEIGHT) {
        b->posy = WORLD_HEIGHT - (*b).rad;
        b->vy = -(*b).vy;
    }   
}

// Randomly initialize a blob. Then set the radius to the provided value.
void BLOB_init(BLOB* b, int rad) {
    BLOB_init(b);
    b->rad = rad;
}

// Randomly initialize a blob. Then set the radius and color to the 
// provided values.
void BLOB_init(BLOB* b, int rad, int color) {
    BLOB_init(b, rad);
    b->color = color;
}

// For debug purposes, you can use this to print a blob's properties to your computer's serial monitor.
void BLOB_print(BLOB b) {
    pc.printf("(%f, %f) <%f, %f> Color: 0x%x\n", b.posx, b.posy, b.vx, b.vy, b.color);
}

// Return the square of the distance from b1 to b2
float BLOB_dist2(BLOB b1, BLOB b2) {
    float difx = b1.posx - b2.posx;
    float dify = b1.posy - b2.posy;
    return (difx * difx) + (dify * dify);
}