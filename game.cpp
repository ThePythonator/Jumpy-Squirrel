#include "game.hpp"
#include "assets.hpp"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120

#define SPRITE_SIZE 8

#define PLAYER_X 32

#define JUMP_VELOCITY 150
#define GRAVITY 450

#define LOG_SPEED 30
#define LOG_SPAWN 100
#define LOG_SPAWN_MIN 60

#define GAP_SIZE 3

using namespace blit;

struct Squirrel {
    float yPosition;
    float yVelocity;

    bool alive;
    bool started;

    int score;
};

struct Log {
    float xPosition;
    int gapPosition;

    int images[SCREEN_HEIGHT / 8];

    bool passed;
};


int state = 0;
double dt;
uint32_t lastTime = 0;

float logSpawnTimer = 0;

Squirrel player;
std::vector<Log> logs;

int get_min_y(int gapPosition) {
    return (gapPosition - GAP_SIZE + 1) * SPRITE_SIZE;
}

int get_max_y(int gapPosition) {
    return (gapPosition + GAP_SIZE) * SPRITE_SIZE;
}

Log generate_log() {
    Log log;
    
    log.gapPosition = (rand() % ((SCREEN_HEIGHT / SPRITE_SIZE) - (GAP_SIZE * 2))) + GAP_SIZE;

    log.xPosition = SCREEN_WIDTH + SPRITE_SIZE;

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
    int index = 10; // need to get animation frame + 10

    //screen.sprite(index, Point(PLAYER_X, player.yPosition));
    screen.rectangle(Rect(PLAYER_X - SPRITE_SIZE / 2, player.yPosition - SPRITE_SIZE / 2, 8, 8));
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

void start_game() {
    //player.alive = true;
    player.yPosition = SCREEN_HEIGHT / 2;
    player.yVelocity = 0;
    player.started = false;
    player.score = 0;
    player.alive = true;
    logSpawnTimer = 0;
    logs.clear();
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

    screen.alpha = 255;
    screen.mask = nullptr;
    screen.pen = Pen(255, 255, 255);

    if (state == 0) {

    }
    else if (state == 1 || state == 2) {
        for (int i = 0; i < logs.size(); i++) {
            render_log(logs.at(i));
        }

        render_player(player);

        screen.text(std::to_string(player.score), minimal_font, Point(SCREEN_WIDTH / 2, 8), true, TextAlign::center_center);
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

            if (player.alive) {
                for (int i = 0; i < logs.size(); i++) {
                    logs.at(i).xPosition -= LOG_SPEED * dt;

                    if (logs.at(i).xPosition < PLAYER_X && !logs.at(i).passed) {
                        // logs.at(i).xPosition + SPRITE_SIZE < PLAYER_X - (SPRITE_SIZE / 2) && !logs.at(i).passed
                        // Note: commented line above only is true if player has completely passed the log (left edge of player is > right edge of log). We may want to give the player a point if they get halfway across log...
                        logs.at(i).passed = true;
                        player.score++;
                    }

                    if ((logs.at(i).xPosition - SPRITE_SIZE) < (PLAYER_X + SPRITE_SIZE / 2) && (logs.at(i).xPosition + SPRITE_SIZE) > (PLAYER_X + SPRITE_SIZE / 2)) {
                        if (get_min_y(logs.at(i).gapPosition) < (player.yPosition - SPRITE_SIZE / 2) && get_max_y(logs.at(i).gapPosition) > (player.yPosition + SPRITE_SIZE / 2)) {
                            
                        }
                        else {
                            player.alive = false;
                            player.yVelocity = 0;
                        }
                    }
                }

                logSpawnTimer -= LOG_SPEED * dt;

                if (logSpawnTimer <= 0) {
                    logs.push_back(generate_log());
                    logSpawnTimer = LOG_SPAWN - player.score * 2;
                    if (logSpawnTimer < LOG_SPAWN_MIN) {
                        logSpawnTimer = LOG_SPAWN_MIN;
                    }
                }

                if (logs.size() > 0) {
                    if (logs.at(0).xPosition < -SPRITE_SIZE) {
                        logs.erase(logs.begin());
                    }
                }
            }

            if (player.yPosition + SPRITE_SIZE / 2 > SCREEN_HEIGHT) {
                player.alive = false;
                state = 2;
                player.yPosition = SCREEN_HEIGHT - SPRITE_SIZE / 2;
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