#include "game.hpp"
#include "assets.hpp"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120

#define SPRITE_SIZE 8

#define PLAYER_X 32

#define PLAYER_UP 20
#define PLAYER_DOWN 20

//                  ^ maybe 30?

#define JUMP_VELOCITY 150
#define GRAVITY 500

#define MAX_VELOCITY 300

#define SCROLL_SPEED 30
#define LOG_SPAWN_GAP 110
#define LOG_MIN_GAP 55

#define GAP_SIZE 3

#define TOLERANCE 1

#define GROUND 5

#define DEAD_TIME 0.4

using namespace blit;

struct SaveData {
    int score;
};

struct Squirrel {
    float yPosition;
    float yVelocity;

    bool alive;
    bool started;
    bool onGround;

    int score;

    float deadTimer;
};

struct Log {
    float xPosition;
    int gapPosition;

    int images[SCREEN_HEIGHT / 8];

    bool passed;
};


int state = 0;
float dt;
uint32_t lastTime = 0;

int localHighscore = 0;

SaveData saveData;

float offset = 0;

int treeNumber = 0;
int treeGap = 0;

Squirrel player;
std::vector<Log> logs;

Surface *background = Surface::load(asset_background);

int get_min_y(int gapPosition) {
    return (gapPosition - GAP_SIZE + 1) * SPRITE_SIZE;
}

int get_max_y(int gapPosition) {
    return (gapPosition + GAP_SIZE) * SPRITE_SIZE;
}

float min(float a, float b) {
    return a < b ? a : b;
}

float max(float a, float b) {
    return a > b ? a : b;
}

Log generate_log() {
    treeNumber++;
    treeGap += max(LOG_SPAWN_GAP - treeNumber * 2, LOG_MIN_GAP);

    Log log;
    
    log.gapPosition = (rand() % ((SCREEN_HEIGHT / SPRITE_SIZE) - (GAP_SIZE * 2))) + GAP_SIZE;

    log.xPosition = treeGap;

    log.passed = false;

    for (int i = 0; i < (SCREEN_HEIGHT / SPRITE_SIZE); i++) {
        if (i == log.gapPosition - GAP_SIZE) {
            log.images[i] = 2;
        }
        else if (i == log.gapPosition + GAP_SIZE) {
            log.images[i] = 0;
        }
        else if (i > log.gapPosition - GAP_SIZE && i < log.gapPosition + GAP_SIZE) {
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

void render_player(Squirrel player) {
    int index = 12; // need to get animation frame

    if (player.onGround) {
        index = 15;
    }
    else if (player.yVelocity < -PLAYER_DOWN) {
        index = 14;
    }
    else if (player.yVelocity > PLAYER_UP) {
        index = 13;
    }
    

    screen.sprite(index, Point(PLAYER_X - SPRITE_SIZE / 2, player.yPosition - SPRITE_SIZE / 2));
    //screen.rectangle(Rect(PLAYER_X - SPRITE_SIZE / 2, player.yPosition - SPRITE_SIZE / 2, 8, 8));
}

void render_log(Log log) {
    for (int i = 0; i < (SCREEN_HEIGHT / SPRITE_SIZE); i++) {
        int index = log.images[i];

        if (index != -1) {
            screen.sprite(index, Point((int)log.xPosition - 8 - offset, i * SPRITE_SIZE));
            screen.sprite(index + 1, Point((int)log.xPosition - offset, i * SPRITE_SIZE));
        }
    }
}

void render_tiles() {
    float littleOffset = offset;
    while (littleOffset >= SPRITE_SIZE) {
        littleOffset -= SPRITE_SIZE;
    }

    for (int i = -1; i < (SCREEN_WIDTH / SPRITE_SIZE) + 1; i++) {
        screen.sprite(10, Point(i * SPRITE_SIZE - littleOffset, SCREEN_HEIGHT - SPRITE_SIZE));
    }
}

void render_title() {
    screen.blit(screen.sprites, Rect(0, 96, 65, 16), Point(0, 0));
    screen.blit(screen.sprites, Rect(0, 112, 76, 16), Point(0, 16));
}

void fade_background() {
    screen.pen = Pen(0, 0, 0, 150);
    screen.clear();
    screen.pen = Pen(255, 255, 255);
}

void start_game() {
    player.yPosition = SCREEN_HEIGHT / 2;
    player.yVelocity = 0;
    player.deadTimer = 0;
    player.started = false;
    player.score = 0;
    player.alive = true;
    player.onGround = false;
    offset = 0;
    treeNumber = 0;
    treeGap = SCREEN_WIDTH / 2;
    logs.clear();
    logs.push_back(generate_log());
}

///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
void init() {
    set_screen_mode(ScreenMode::lores);
    screen.sprites = Surface::load(asset_sprites);

    // Attempt to load the first save slot.
    if (read_save(saveData)) {
        // Loaded sucessfully!
        localHighscore = saveData.score;
    }
    else {
        // No save file or it failed to load, set up some defaults.
        saveData.score = 0;
    }
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

    screen.alpha = 255;
    screen.mask = nullptr;
    screen.pen = Pen(255, 255, 255);

    screen.blit(background, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), Point(0, 0), false);

    if (state == 0) {
        render_tiles();

        fade_background();

        //render_title();

        screen.text("Jumpy Squirrel", minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 1 / 3), true, TextAlign::center_center); // change to custom icon

        screen.text("Press A to Start", minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 2 / 3), true, TextAlign::center_center);
    }
    else if (state == 1) {
        render_tiles();

        for (int i = 0; i < logs.size(); i++) {
            render_log(logs.at(i));
        }

        render_player(player);

        screen.text(std::to_string(player.score), minimal_font, Point(SCREEN_WIDTH / 2, 8), true, TextAlign::center_center);
    }
    else if (state == 2) {
        render_tiles();

        for (int i = 0; i < logs.size(); i++) {
            render_log(logs.at(i));
        }

        render_player(player);

        fade_background();

        screen.text("Score: "+std::to_string(player.score), minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 1 / 3), true, TextAlign::center_center);

        screen.text("Highscore: " + std::to_string(localHighscore), minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 1 / 2), true, TextAlign::center_center);

        screen.text("Press A to Retry", minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 2 / 3), true, TextAlign::center_center);
    }

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
    dt = (time - lastTime) / 1000.0;
    lastTime = time;

    if (state == 0) {
        if (buttons.pressed & Button::A) {
            state = 1;
            start_game();
        }
    }
    else if (state == 1) {
        if (player.alive) {
            if ((buttons.pressed & Button::A)) {
                player.yVelocity = JUMP_VELOCITY;
                player.started = true;
            }
        }
        

        if (player.started) {
            player.yVelocity -= GRAVITY * dt;

            player.yPosition -= player.yVelocity * dt;
            player.yVelocity = max(player.yVelocity, -MAX_VELOCITY);

            if (player.alive) {
                offset += SCROLL_SPEED * dt;

                for (int i = 0; i < logs.size(); i++) {

                    if (logs.at(i).xPosition - offset < PLAYER_X && !logs.at(i).passed) {
                        // logs.at(i).xPosition + SPRITE_SIZE < PLAYER_X - (SPRITE_SIZE / 2) && !logs.at(i).passed
                        // Note: commented line above only is true if player has completely passed the log (left edge of player is > right edge of log). We may want to give the player a point if they get halfway across log...
                        logs.at(i).passed = true;
                        player.score++;
                    }

                    if ((logs.at(i).xPosition - SPRITE_SIZE - offset) < (PLAYER_X + SPRITE_SIZE / 2) && (logs.at(i).xPosition + SPRITE_SIZE - offset - TOLERANCE) > (PLAYER_X - SPRITE_SIZE / 2)) {
                        if (get_min_y(logs.at(i).gapPosition) - TOLERANCE < (player.yPosition - SPRITE_SIZE / 2) && get_max_y(logs.at(i).gapPosition) + TOLERANCE > (player.yPosition + SPRITE_SIZE / 2)) {
                            
                        }
                        else {
                            player.alive = false;
                            player.yVelocity = 0;
                        }
                    }
                }

                if (logs.at(logs.size() - 1).xPosition - offset < SCREEN_WIDTH + SPRITE_SIZE) {
                    logs.push_back(generate_log());
                }

                if (logs.size() > 0) {
                    if (logs.at(0).xPosition < -SPRITE_SIZE) {
                        logs.erase(logs.begin());
                    }
                }
            }


            if (player.yPosition + (SPRITE_SIZE / 2) > SCREEN_HEIGHT - GROUND) {
                player.alive = false;
                player.yPosition = SCREEN_HEIGHT - (SPRITE_SIZE / 2) - GROUND;
                player.onGround = true;
                player.deadTimer += dt;
            }

            if (player.deadTimer >= DEAD_TIME) {
                state = 2;
                localHighscore = max(localHighscore, player.score);
                saveData.score = localHighscore;
                write_save(saveData); // write highscore
            }
        }
    }
    else if (state == 2) {
        if (buttons.pressed & Button::A) {
            state = 1;
            start_game();
        }
    }
}