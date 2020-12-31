#include "game.hpp"
#include "assets.hpp"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120

#define SPRITE_SIZE 8

using namespace blit;

struct Squirrel {
    float yPosition;
    float yVelocity;

    int score;
};

struct Log {
    float xPosition;
    int gapPosition;

    int images[SCREEN_HEIGHT / 8];
};

Log generate_log() {
    Log log;
    
    log.gapPosition = (rand() % ((SCREEN_HEIGHT / SPRITE_SIZE) - 4)) + 2; // need to check this is correct.... maybe bounds are not right

    /*for (int x = 0; x < 100; x++) {
        log.gapPosition = (rand() % ((SCREEN_HEIGHT / SPRITE_SIZE) - 4)) + 2;
        printf("%d\n", log.gapPosition);
    }*/
    

    log.xPosition = SCREEN_WIDTH + SPRITE_SIZE;

    for (int i = 0; i < (SCREEN_HEIGHT / SPRITE_SIZE); i++) {
        if (i == log.gapPosition - 2) {
            log.images[i] = 2;
        }
        else if (i == log.gapPosition + 2) {
            log.images[i] = 0;
        }
        else if (i > log.gapPosition - 2 && i < log.gapPosition + 2) {
            log.images[i] = -1;
        }
        else {
            if (rand() % 4 == 0) {
                log.images[i] = (rand() % 2) * 2 + 6;
            }
            else {
                log.images[i] = 4;
            }
            
        }
    }

    return log;
}

void render_log(Log log) {
    for (int i = 0; i < (SCREEN_HEIGHT / SPRITE_SIZE); i++) {
        int index = log.images[i];

        if (index != -1) {
            screen.sprite(index, Point(log.xPosition - 8, i * SPRITE_SIZE));
            screen.sprite(index + 1, Point(log.xPosition, i * SPRITE_SIZE));
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
void init() {
    set_screen_mode(ScreenMode::lores);
    screen.sprites = SpriteSheet::load(asset_sprites);
}

///////////////////////////////////////////////////////////////////////////
//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//
void render(uint32_t time) {

    // clear the screen -- screen is a reference to the frame buffer and can be used to draw all things with the 32blit
    screen.clear();

    // draw some text at the top of the screen
    screen.alpha = 255;
    screen.mask = nullptr;
    screen.pen = Pen(255, 255, 255);

    static Log log1 = generate_log();
    static Log log2 = generate_log();
    log1.xPosition = 60;
    log2.xPosition = 100;

    render_log(log1);
    render_log(log2);

    screen.pen = Pen(0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////
//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//
void update(uint32_t time) {
}