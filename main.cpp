// Student Side Shell Code
// For the baseline, anywhere you see ***, you have code to write.

#include "mbed.h"
#include "playSound.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "game_synchronizer.h"
#include "misc.h"
#include "blob.h"

#define NUM_BLOBS 22

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalIn pb_u(p21);                        // Up Button
DigitalIn pb_r(p22);                        // Right Button
DigitalIn pb_d(p23);                        // Down Button
DigitalIn pb_l(p24);                        // Left Button

Serial pc(USBTX, USBRX);                    // Serial connection to PC. Useful for debugging!
MMA8452 acc(p28, p27, 100000);              // Accelerometer (SDA, SCL, Baudrate)
uLCD_4DGL uLCD(p9,p10,p11);                 // LCD (tx, rx, reset)
SDFileSystem sd(p5, p6, p7, p8, "sd");      // SD  (mosi, miso, sck, cs)
AnalogOut DACout(p18);                      // speaker
wave_player player(&DACout);                // wav player
GSYNC game_synchronizer;                    // Game_Synchronizer
GSYNC* sync = &game_synchronizer;           //
Timer frame_timer;                          // Timer

int score1 = 0;                             // Player 1's score.
int score2 = 0;                             // Player 2's score.


// ***
// Display a pretty game menu on the player 1 mbed. 
// Do a good job, and make it look nice. Give the player
// an option to play in single- or multi-player mode. 
// Use the buttons to allow them to choose.
int game_menu(void) {
    
    uLCD.background_color(BGRD_COL);
    uLCD.textbackground_color(BGRD_COL);
    uLCD.cls();
    uLCD.baudrate(3000000);
    uLCD.printf("Welcome to Agar!\n\n");
    uLCD.printf("Up: Single Player\n");
    uLCD.printf("Down: Multi-Player");
    char buzzer[] = "/sd/wavfiles/game_opening.wav";
    playSound(buzzer);
    
    // ***
    // Spin until a button is pressed. Depending which button is pressed, 
    // return either SINGLE_PLAYER or MULTI_PLAYER.
    while(1) {
        if(!pb_u) { 
            // Add difficulty levels here
            return SINGLE_PLAYER;
        }
        else if(!pb_d) {
            // Add difficulty levels here
            return MULTI_PLAYER; 
        }   
    }
}

// Initialize the game hardware. 
// Call game_menu to find out which mode to play the game in (Single- or Multi-Player)
// Initialize the game synchronizer.
void game_init(void) {
    
    led1 = 0; led2 = 0; led3 = 0; led4 = 0;
    
    pb_u.mode(PullUp);
    pb_r.mode(PullUp); 
    pb_d.mode(PullUp);    
    pb_l.mode(PullUp);
    
    pc.printf("\033[2J\033[0;0H");              // Clear the terminal screen.
    pc.printf("I'm alive! Player 1\n");         // Let us know you made it this far

    // game_menu MUST return either SINGLE_PLAYER or MULTI_PLAYER
    int num_players = game_menu();
    
    GS_init(sync, &uLCD, &acc, &pb_u, &pb_r, &pb_d, &pb_l, num_players, PLAYER1); // Connect to the other player.
        
    pc.printf("Initialized...\n");              // Let us know you finished initializing.
    srand(time(NULL));                          // Seed the random number generator.

    GS_cls(sync, SCREEN_BOTH);
    GS_update(sync);
}

// Display who won!
void game_over(int winner) {
    if((*sync).play_mode == SINGLE_PLAYER) {    // Clear the screen
        GS_cls(sync, SCREEN_P1);                
    } else {
        GS_cls(sync, SCREEN_BOTH);
    }
    char buzzer[] = "/sd/wavfiles/game_over.wav";
    playSound(buzzer);
    // Pause forever
    while(1) {
        if (winner == WINNER_P1) {              // Screen for Player 1 Win
            if((*sync).play_mode == SINGLE_PLAYER) {
                GS_locate(sync, SCREEN_P1, 1, 1);
                GS_puts(sync, SCREEN_P1, "Player 1 Wins!", 15);
            } else {
                GS_locate(sync, SCREEN_BOTH, 1, 1);
                GS_puts(sync, SCREEN_BOTH, "Player 1 Wins!", 15);
            }
            GS_update(sync);
        } else if (winner == WINNER_P2) {       // Screen for Player 2 Win
            GS_locate(sync, SCREEN_BOTH, 1, 1);
            GS_puts(sync, SCREEN_BOTH, "Player 2 Wins!", 15);
            GS_update(sync);
        } else if (winner == WINNER_TIE) {      // Screen for draw ending
            GS_locate(sync, SCREEN_BOTH, 1, 1);
            GS_puts(sync, SCREEN_BOTH, "It's a draw..", 14); 
            GS_update(sync);
        }    
    }
}

// Take in a pointer to the blobs array. Iterate over the array
// and initialize each blob with BLOB_init(). Set the first blob to (for example) blue
// and the second blob to (for example) red. Set the color(s) of the food blobs however you like.
// Make the radius of the "player blobs" equal and larger than the radius of the "food blobs".
void generate_blobs(BLOB* blobs) {
    // ***
     
    // Initializes first blob (Player1)
    BLOB_init(blobs, 10, P1_COL);
    
    // Initializes second blob (Player2)
    BLOB_init((blobs + 1), 10, P2_COL);
        
    // Initialzes food blobs
    int i;
    for(i = 2; i < 22; i++) {
        BLOB_init((blobs+i), 5);    
    }
}

int main (void) {
    
    int* p1_buttons;
    int* p2_buttons;
    
    float ax1, ay1, az1;
    float ax2, ay2, az2;
    
    // Ask the user to choose (via pushbuttons)
    // to play in single- or multi-player mode.
    // Use their choice to correctly initialize the game synchronizer.
    game_init();  
    
    // Keep an array of blobs. Use blob 0 for player 1 and
    // blob 1 for player 2.
    BLOB blobs[NUM_BLOBS];
        
    // Pass in a pointer to the blobs array. Iterate over the array
    // and initialize each blob with BLOB_init(). Set the radii and colors
    // anyway you see fit.
    generate_blobs(blobs);

    while(true) {
        
        // In single-player, check to see if the player has eaten all other blobs.
        // In multi-player mode, check to see if the players have tied by eating half the food each.
        // ***
        
        // Single Player
        if ((*sync).play_mode == SINGLE_PLAYER) {
            if (score1 == 21) {
                game_over(WINNER_P1);
            }
        }
        
        // Multi-Player
        else if ((*sync).play_mode == MULTI_PLAYER) {
            if (score1 == score2 == 10) {
                game_over(WINNER_TIE);    
            }
        }
        
        // In both single- and multi-player modes, display the score(s) in an appropriate manner.
        // ***
        // * use sprintf to format string
        GS_locate(sync, SCREEN_P1, 1, 1);
        char text1[15];
        int n1 = sprintf(text1, "P1 Score: %d", score1);
        GS_puts(sync, SCREEN_P1, text1, n1+1);
        if ((*sync).play_mode == MULTI_PLAYER) {
            GS_locate(sync, SCREEN_P2, 1, 1);
            char text2[15];
            int n2 = sprintf(text2, "P2 Score: %d", score2);
            GS_puts(sync, SCREEN_P2, text2, n2+1);   
        }
        
        // Use the game synchronizer API to get the button values from both players' mbeds.
        p1_buttons = GS_get_p1_buttons(sync);
        p2_buttons = GS_get_p2_buttons(sync);
        
        // Use the game synchronizer API to get the accelerometer values from both players' mbeds.
        GS_get_p1_accel_data(sync, &ax1, &ay1, &az1);
        GS_get_p2_accel_data(sync, &ax2, &ay2, &az2);
        
        // If the magnitude of the p1 x and/or y accelerometer values exceed ACC_THRESHOLD,
        // set the blob 0 velocities to be proportional to the accelerometer values.
        // ***
        if(abs(ax1) > ACC_THRESHOLD){
            blobs[0].vx = -ax1 * 10000 / blobs[0].rad;      // Update x velocity for Player 1   
        } else if (abs(ay1) > ACC_THRESHOLD) {
            blobs[0].vy = ay1 * 10000 / blobs[0].rad;       // Update y velocity for Player 1
        }
        
        // If in multi-player mode and the magnitude of the p2 x and/or y accelerometer values exceed ACC_THRESHOLD,
        // set the blob 0 velocities to be proportional to the accelerometer values.
        // ***
        if ((*sync).play_mode == MULTI_PLAYER) {
            if(abs(ax2) > ACC_THRESHOLD){
                blobs[1].vx = -ax2 * 10000 / blobs[1].rad;      // Update x velocity for Player 2
            } else if (abs(ay2) > ACC_THRESHOLD) {
                blobs[1].vy = ay2 * 10000 / blobs[1].rad;       // Update y velocity for Player 2
            }    
        }
        
        float time_step = .01; 
        
        // Undraw the world bounding rectangle (use BGRD_COL).
        // ***
        GS_rectangle(sync, SCREEN_P1, -(blobs[0].posx), -(blobs[0].posy), (WORLD_WIDTH - blobs[0].posx), (WORLD_HEIGHT - blobs[0].posy), BGRD_COL);
        if ((*sync).play_mode == MULTI_PLAYER) {
            GS_rectangle(sync, SCREEN_P2, -(blobs[1].posx), -(blobs[1].posy), (WORLD_WIDTH - blobs[1].posx), (WORLD_HEIGHT - blobs[1].posy), BGRD_COL);    
        }
        
        // Loop over all blobs
        // ***
        int i;
        for(i = 0; i < NUM_BLOBS; i++) {  
    
            // If the current blob is valid, or it was deleted in the last frame, (delete_now is true), then draw a background colored circle over
            // the old position of the blob. (If delete_now is true, reset delete_now to false.)  
            // ***
            if(blobs[i].valid) {
                GS_filled_circle(sync, SCREEN_P1, blobs[i].old_x - blobs[0].old_x, blobs[i].old_y - blobs[0].old_y, blobs[i].rad+2, BGRD_COL);
                if ((*sync).play_mode == MULTI_PLAYER) {
                    GS_filled_circle(sync, SCREEN_P2, (blobs[i].old_x - blobs[1].old_x), (blobs[i].old_y - blobs[1].old_y), blobs[i].rad+2, BGRD_COL);
                }        
            } else if (blobs[i].delete_now) {
                GS_filled_circle(sync, SCREEN_P1, blobs[i].old_x - blobs[0].old_x, blobs[i].old_y - blobs[0].old_y, blobs[i].rad+2, BGRD_COL);
                if ((*sync).play_mode == MULTI_PLAYER) {
                    GS_filled_circle(sync, SCREEN_P2, blobs[i].old_x - blobs[1].old_x, blobs[i].old_y - blobs[1].old_y, blobs[i].rad+2, BGRD_COL);
                }
                blobs[i].delete_now = false;
            }
            
            // Use the blob positions and velocities, as well as the time_step to compute the new position of the blob.
            // ***
            blobs[i].old_x = blobs[i].posx;
            blobs[i].posx = blobs[i].posx + (blobs[i].vx * time_step);
            blobs[i].old_y = blobs[i].posy;
            blobs[i].posy = blobs[i].posy + (blobs[i].vy * time_step);
            
            // If the current blob is blob 0, iterate over all blobs and check if they are close enough to eat or be eaten. 
            // In multi-player mode, if the player 0 blob is eaten, player 1 wins and vise versa.        
            // If blob 0 eats some food, increment score1.   
            // ***
            if (i == 0) {
                int j;
                for (j = 1; j < 22; j++) {          // Iterates over all blobs other than Player 1
                    if (blobs[j].valid) {
                        if (sqrt(BLOB_dist2(blobs[i], blobs[j])) < (blobs[i].rad + blobs[j].rad)) {     // If the blobs are past touching
                            if (blobs[i].rad > blobs[j].rad) {                                      // If Player 1 radius > blob, blob gets eaten
                                if (j == 1 && (*sync).play_mode == MULTI_PLAYER) {
                                    char buzzer[] = "/sd/wavfiles/player2player.wav";
                                    playSound(buzzer);
                                    game_over(WINNER_P1);    
                                } else if (j == 1 && (*sync).play_mode == SINGLE_PLAYER){
                                    char buzzer[] = "/sd/wavfiles/player2player.wav";
                                    playSound(buzzer);
                                    score1++;
                                    blobs[j].delete_now = true;    
                                    blobs[j].valid = false;
                                    blobs[i].rad += .5;
                                } else {
                                    char buzzer[] = "/sd/wavfiles/eat.wav";
                                    playSound(buzzer);
                                    score1++;                       // Increment score
                                    blobs[j].delete_now = true;    
                                    blobs[j].valid = false;
                                    blobs[i].rad += .5;             // Make the blob bigger
                                }
                            } else if (blobs[j].rad > blobs[i].rad) {               // If Player 2 is bigger than Player 1, he/she wins
                                if  (j == 1 && (*sync).play_mode == MULTI_PLAYER) {
                                    char buzzer[] = "/sd/wavfiles/player2player.wav";
                                    playSound(buzzer);
                                    game_over(WINNER_P2);    
                                } 
                            }
                        }
                    } 
                }    
            }
            
            // If the current blob is blob 1 and we are playing in multi-player mode, iterate over all blobs and check
            // if they are close enough to eat or be eaten. In multi-player mode, if the player 1 blob is eaten, player 0 wins and vise versa.
            // If blob1 eats some food, increment score2.
            // ***
            if (i == 1 && (*sync).play_mode == MULTI_PLAYER) {
                int k;
                for(k = 2; k < 22; k++) {
                    if (blobs[k].valid) {
                        if (sqrt(BLOB_dist2(blobs[i], blobs[k])) < (pow(blobs[i].rad,2) + pow(blobs[k].rad,2))) {
                            char buzzer[] = "/sd/wavfiles/eat.wav";
                            playSound(buzzer);
                            score2++;                       // Increment score
                            blobs[k].delete_now = true;
                            blobs[k].valid = false;
                            blobs[i].rad += .5;             // Make the blob bigger
                        }
                    }  
                }   
            } 
            
            // You have to implement this function.
            // It should take in a pointer to a blob and constrain that blob to the world.
            // More details in blob.cpp
            BLOB_constrain2world(&blobs[i]);           
            
        }
        // Iterate over all blobs and draw them at their newly computed positions. Reference their positions to the player blobs.
        // That is, on screen 1, center the world on blob 0 and reference all blobs' position to that of blob 0.
        // On screen 2, center the world on blob 1 and reference all blobs' position tho that of blob 1.
        // ***
        for (i = 0; i < 22; i++) {
            if (blobs[i].valid) {
                GS_circle(sync, SCREEN_P1, blobs[i].posx - blobs[0].posx, blobs[i].posy - blobs[0].posy, blobs[i].rad, blobs[i].color);
                if ((*sync).play_mode == MULTI_PLAYER) {
                    GS_circle(sync, SCREEN_P2, (blobs[i].posx - blobs[1].posx), (blobs[i].posy - blobs[1].posy), blobs[i].rad, blobs[i].color);
                }
            }
        }
        
        // Redraw the world boundary rectangle.
        // ***
        GS_rectangle(sync, SCREEN_P1, -(blobs[0].posx), -(blobs[0].posy), (WORLD_WIDTH - blobs[0].posx), (WORLD_HEIGHT - blobs[0].posy), BORDER_COL);
        if ((*sync).play_mode == MULTI_PLAYER) {
            GS_rectangle(sync, SCREEN_P2, -(blobs[1].posx), -(blobs[1].posy), (WORLD_WIDTH - blobs[1].posx), (WORLD_HEIGHT - blobs[1].posy), BORDER_COL);    
        }
        
        // Update the screens by calling GS_update.
        GS_update(sync);
        
        // If a button on either side is pressed, the corresponding LED on the player 1 mbed will toggle.
        // This is just for debug purposes, and to know that your button pushes are being registered.
        led1 = p1_buttons[0] ^ p2_buttons[0];
        led2 = p1_buttons[1] ^ p2_buttons[1];
        led3 = p1_buttons[2] ^ p2_buttons[2];
        led4 = p1_buttons[3] ^ p2_buttons[3];
    } 
}