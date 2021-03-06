#include <nds.h>
#include <maxmod9.h>
#include <stdio.h>

#include "ball.h"
#include "geometry.h"
#include "player.h"
#include "stats.h"
#include "audio.h"

// Audio
#include "soundbank.h"


// Methods

void initBall(ball *b) {
    b->speed = 2;
    b->box.pos.x = 100;
    b->box.pos.y = 30;
    b->box.width = 10;
    b->box.height = 10;
    b->direction.x = 1;
    b->direction.y = 1;

    // SFX_WALL
    b->sfx_wall.id = SFX_WALL;
    b->sfx_wall.rate = 0x400/2;
    b->sfx_wall.handle = 0;
    b->sfx_wall.volume = 255;
    b->sfx_wall.panning = 128;

    // SFX_PANEL
    b->sfx_panel.id = SFX_PANEL;
    b->sfx_panel.rate = 0x400/2;
    b->sfx_panel.handle = 0;
    b->sfx_panel.volume = 255;
    b->sfx_panel.panning = 128;

    // SFX_READY
    b->sfx_ready.id = SFX_READY;
    b->sfx_ready.rate = 0x400/2;
    b->sfx_ready.handle = 0;
    b->sfx_ready.volume = 255;
    b->sfx_ready.panning = 128;
}

/**
 * @param ball b
 * @return void
 */
void moveBall(ball *b, player *p1, player *p2, scoreBox *sBox) {
    b->box.pos.x += b->direction.x * b->speed;
    b->box.pos.y += b->direction.y * b->speed;

    bool hitLeftPaddle = intersect(b->box, p1->box);
    bool hitRightPaddle = intersect(b->box, p2->box);

    // horizontal collision
    if (b->box.pos.x <= 0) {
       scoring(1, b, sBox); 
    } else if (b->box.pos.x + b->box.width >= SCREEN_WIDTH) {
       scoring(0, b, sBox); 
    } else if (hitLeftPaddle || hitRightPaddle) {
        // calculate relative position to player box, then set direction according to that relative position
        // if relative position is low (i.e. closer to 0, the ball hit the paddle early),
        // then the ball goes back in the direction it came from
        // if relative position is high (i.e. closer to 1, the ball hit the paddle late),
        // then the ball bounces off the paddle, going off in the other direction
        // if relative position is around 0.5 (i.e. the ball hit the paddle in the middle),
        // then the ball goes back from the middle of the paddle
        float relativePos;
        if (hitLeftPaddle) {
            // player 1
            relativePos = (b->box.pos.y - p1->box.pos.y) / (p1->box.height);
        } else {
            // player 2
            relativePos = (b->box.pos.y - p2->box.pos.y) / (p2->box.height);
        }
        // y is negative, so the ball is flying up: reverse relative position
        if (b->direction.y < 0) {
            relativePos = 1 - relativePos;
        }

        // new x direction, reverse the sign
        b->direction.x *= -1;

        // new y direction, keep the sign
        b->direction.y = (b->direction.y > 0) ? 1 : -1;
        b->direction.y *= -0.5 + 1.5 * relativePos;

        // position ball so that it does not intersect the paddles anymore
        if (hitLeftPaddle) {
            b->box.pos.x = p1->box.pos.x + p1->box.width;
        } else {
            b->box.pos.x = p2->box.pos.x - b->box.width;
        }

        // increase speed of ball
        if (b->speed < 5) {
            b->speed += 0.1;
        }

        // sound effect
        mmEffectEx( &b->sfx_panel );
    }
    
    // vertical collision
    if (b->box.pos.y <= 0 || b->box.pos.y + b->box.height >= SCREEN_HEIGHT) {
        b->direction.y *= -1;
        mmEffectEx( &b->sfx_wall );
    }


    // Updating sprite position
    b->sprite->x = (int)b->box.pos.x;
    b->sprite->y = (int)b->box.pos.y;
}

void scoring(int player, ball *b, scoreBox *sBox) {
    // reset ball to middle of screen
    b->box.pos.x = SCREEN_WIDTH / 2;
    b->box.pos.y = SCREEN_HEIGHT / 2;
    b->direction.x = (b->direction.x > 0) ? -1 : 1;
    b->direction.y = 1;
    b->speed = 2;
    
    countPoint(sBox, player);    
    
    //mmEffectEx( &b->sfx_ready );
}
