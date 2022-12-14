#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "audio.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// Define screen dimensions
#define SCREEN_WIDTH    200
#define SCREEN_HEIGHT   200
#define FULL_SCREEN     false

// Define Tunisia flag image path
#define SPACESHIP_IMAGE_PATH "assets/spaceship.png"
#define CHROME_IMAGE_PATH "assets/chrome.png"
#define FIREFOX_IMAGE_PATH "assets/firefox.png"

struct sprite{
    int vx;
    int vy;
    SDL_Rect rect;
    SDL_Texture *texture;
    int speed;
    bool die;
};

bool rect_collision_detection(struct sprite a,struct sprite b){
    if (a.rect.x < b.rect.x + b.rect.w &&
        a.rect.x + a.rect.w > b.rect.x &&
        a.rect.y < b.rect.y + b.rect.h &&
        a.rect.h + a.rect.y > b.rect.y) {
        // collision detected!
        return true;
    }else{
        return false;
    }
}

bool circle_collision_detection(struct sprite a,struct sprite b){
    float dx = (a.rect.x + a.rect.w/2) - (b.rect.x + b.rect.w/2);
    float dy = (a.rect.y + a.rect.w/2) - (b.rect.y + b.rect.w/2);
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < a.rect.w/2 + b.rect.w/2) {
        // collision detected!
        return true;
    } else {
        // no collision
        return false;
    }
}

static int playSoundThread(void *path){
    playSound(path, SDL_MIX_MAXVOLUME);
    return 1;
}

static void playAudio(char *path){
    SDL_Thread *thread;
    int threadReturnValue;
    thread = SDL_CreateThread(playSoundThread, "TestThread",path);
    SDL_WaitThread(thread, &threadReturnValue);
}

int main(int argc, char* argv[]){
    // Unused argc, argv
    (void) argc;
    (void) argv;

    // Initialize SDL2
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
        printf("SDL2 could not be initialized!\n"
               "SDL2 Error: %s\n", SDL_GetError());
        return 0;
    }

    /* Initialize Simple-SDL2-Audio */
    initAudio();

    // Initialize SDL2_image
    int flags = IMG_INIT_PNG; // Can be: IMG_INIT_JPG | IMG_INIT_PNG
    if((IMG_Init(flags) & flags) != flags) {
        printf("SDL2_image could not be initialized with PNG support!\n"
               "SDL2_image Error: %s\n", IMG_GetError());
        return 0;
    }

    /* Initialize SDL2_TTF */
    TTF_Init();

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
    // Disable compositor bypass
    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")){
        printf("SDL can not disable compositor bypass!\n");
        return 0;
    }
#endif

    // Create window
    SDL_Window *window = SDL_CreateWindow("space war",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if(FULL_SCREEN){
        SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN);
    }
    

    if(!window){
        printf("Window could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    }else{
        // Create renderer
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if(!renderer){
            printf("Renderer could not be created!\n"
                   "SDL_Error: %s\n", SDL_GetError());
        }else{
            const int FPS = 60;
            const int frameDelay = 1000 / FPS;
            Uint32 frameStart;
            int frameTime;
            
            struct sprite enemys[10];
            struct sprite fires[10];

            for(int i = 0; i < 10; i++ ){
                enemys[i].texture = IMG_LoadTexture(renderer, CHROME_IMAGE_PATH);
                enemys[i].rect.x=50*i;
                enemys[i].rect.y=10;
                enemys[i].rect.w=40;
                enemys[i].rect.h=40;
                enemys[i].vx=0;
                enemys[i].vy=1;
                enemys[i].speed=1;
                enemys[i].die = false;
            }

            for(int i = 0; i < 10; i++ ){
                fires[i].texture = IMG_LoadTexture(renderer, FIREFOX_IMAGE_PATH);
                fires[i].rect.x=0;
                fires[i].rect.y=0;
                fires[i].rect.w=5;
                fires[i].rect.h=5;
                fires[i].vx=0;
                fires[i].vy=-2;
                fires[i].speed=2;
                fires[i].die = true;
            }

            struct sprite player;

            player.texture = IMG_LoadTexture(renderer, SPACESHIP_IMAGE_PATH);
            player.speed = 2;
            player.vx=0;
            player.vy=0;
            player.rect.x=75;
            player.rect.y=150;
            player.rect.w=50;
            player.rect.h=50;
            player.die = false;

            // Event loop exit flag
            bool quit = false;

            int time = 0;

            int score = 0;

            // init score
            TTF_Font * font = TTF_OpenFont("assets/font.ttf", 25);
            SDL_Color color = {255,255,255,255};
            SDL_Surface * score_surface = TTF_RenderText_Solid(font,"0", color);
            SDL_Texture * score_texture = SDL_CreateTextureFromSurface(renderer, score_surface);
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(score_texture, NULL, NULL, &texW, &texH);
            SDL_Rect score_rect;
            score_rect.x=10;
            score_rect.y=10;
            score_rect.w=texW;
            score_rect.h=texH;

            // gameover text
            TTF_Font * gameover_font = TTF_OpenFont("assets/font.ttf", 40);
            SDL_Color gameover_color = {255,0,0,255};
            SDL_Surface * gameover_surface = TTF_RenderText_Solid(gameover_font,"GAME OVER", gameover_color);
            SDL_Texture * gameover_texture = SDL_CreateTextureFromSurface(renderer, gameover_surface);

            int gameoverW = 0;
            int gameoverH = 0;
            SDL_QueryTexture(gameover_texture, NULL, NULL, &gameoverW, &gameoverH);
            SDL_Rect gameover_rect;
            gameover_rect.x=SCREEN_WIDTH/2-gameoverW/2;
            gameover_rect.y=SCREEN_HEIGHT/2-gameoverH/2;
            gameover_rect.w=gameoverW;
            gameover_rect.h=gameoverH;
            
            // SDL_DestroyTexture(score_texture);
            // SDL_FreeSurface(score_surface);

            // Event loop

            bool gameover = false;

            while(!quit){
                SDL_Event event;

                while( SDL_PollEvent( &event ) != 0 ){
                    if(event.type == SDL_QUIT){
                        quit = true;
                    }


                    if(gameover){
                        switch( event.type ){
                            case SDL_KEYDOWN:
                                switch (event.key.keysym.sym){
                                    //https://wiki.libsdl.org/SDL_Keycode
                                    case SDLK_r:
                                        //restart game
                                        //score reset 0
                                        score = 0;
                                        char score_text[10];
                                        sprintf(score_text,"%d",score);
                                        score_surface = TTF_RenderText_Solid(font,score_text, color);
                                        score_texture = SDL_CreateTextureFromSurface(renderer, score_surface);
                                        int texW = 0;
                                        int texH = 0;
                                        SDL_QueryTexture(score_texture, NULL, NULL, &texW, &texH);
                                        score_rect.w=texW;
                                        score_rect.h=texH;
                                        //clear fires
                                        for(int i = 0; i < 10; i++ ){
                                            fires[i].die=true;
                                        }
                                        //reset enemys
                                        for(int i = 0; i < 10; i++ ){
                                            enemys[i].texture = IMG_LoadTexture(renderer, CHROME_IMAGE_PATH);
                                            enemys[i].rect.x=50*i;
                                            enemys[i].rect.y=10;
                                            enemys[i].rect.w=40;
                                            enemys[i].rect.h=40;
                                            enemys[i].vx=0;
                                            enemys[i].vy=1;
                                            enemys[i].speed=1;
                                            enemys[i].die = false;
                                        }
                                        //reset gameover status
                                        gameover=false;
                                    break;
                                }
                                break;
                        }
                    }else{
                        switch( event.type ){
                            case SDL_KEYDOWN:
                                switch (event.key.keysym.sym){
                                    //https://wiki.libsdl.org/SDL_Keycode
                                    case SDLK_a:
                                        player.vy=0;
                                        player.vx=-1*player.speed;
                                    break;
                                    case SDLK_d:
                                        player.vy=0;
                                        player.vx=1*player.speed;
                                    break;
                                    case SDLK_w:
                                        player.vx=0;
                                        player.vy=-1*player.speed;
                                    break;
                                    case SDLK_s:
                                        player.vx=0;
                                        player.vy=1*player.speed;
                                    break;
                                    case SDLK_SPACE:
                                        for(int i = 0; i < 10; i++ ){
                                            if(fires[i].die){
                                                fires[i].rect.x=player.rect.x+player.rect.w/2-fires[i].rect.w/2;
                                                fires[i].rect.y=player.rect.y;
                                                fires[i].die=false;
                                                playAudio("assets/gan.wav");
                                                break;
                                            }
                                        }
                                        break;
                                }
                                break;

                            case SDL_KEYUP:
                                switch (event.key.keysym.sym){
                                    //https://wiki.libsdl.org/SDL_Keycode
                                    case SDLK_a:
                                        player.vx=0;
                                    break;
                                    case SDLK_d:
                                        player.vx=0;
                                    break;
                                    case SDLK_w:
                                        player.vy=0;
                                    break;
                                    case SDLK_s:
                                        player.vy=0;
                                    break;
                                    case SDLK_SPACE:
                                        break;
                                }
                                break;

                            default:
                                break;
                        }
                    }
                }

                frameStart = SDL_GetTicks();
                
                // Initialize renderer color white for the background
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

                // Clear screen
                SDL_RenderClear(renderer);

                // Set renderer color black to draw the square
                // SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

                // Draw filled square
                //SDL_RenderFillRect(renderer, &squareRect);

                //update

                time += 1;

                if(time % 10 ==0){
                    
                }

                if(!gameover){
                    player.rect.x+=player.vx;
                    player.rect.y+=player.vy;

                    // if enemy die then init
                    for(int i = 0; i < 10; i++ ){
                        if(enemys[i].die){
                            enemys[i].rect.y=-100;
                            enemys[i].die=false;
                        }
                    }

                    for(int i = 0; i < 10; i++ ){
                        if(!enemys[i].die){
                            enemys[i].rect.y+=enemys[i].vy;

                            if(circle_collision_detection(enemys[i],player)){
                                gameover = true;
                            }

                            if(enemys[i].rect.y>SCREEN_HEIGHT){
                                enemys[i].die=true;
                            }
                        }
                    }

                    for(int i = 0; i < 10; i++ ){
                        if(!fires[i].die){
                            fires[i].rect.y+=fires[i].vy;
                            if(fires[i].rect.y<-1*fires[i].rect.h){
                                fires[i].die=true;
                            }else{
                                //
                                for(int j = 0; j < 10; j++ ){
                                    if(!enemys[j].die){
                                        if(circle_collision_detection(fires[i],enemys[j])){
                                            
                                            playAudio("assets/blast.wav");

                                            //update score begin
                                            score += 1;
                                            char score_text[10];
                                            sprintf(score_text,"%d",score);
                                            score_surface = TTF_RenderText_Solid(font,score_text, color);
                                            score_texture = SDL_CreateTextureFromSurface(renderer, score_surface);
                                            int texW = 0;
                                            int texH = 0;
                                            SDL_QueryTexture(score_texture, NULL, NULL, &texW, &texH);
                                            score_rect.w=texW;
                                            score_rect.h=texH;
                                            //update score end

                                            fires[i].die=true;
                                            enemys[j].die=true;
                                        }
                                    }
                                }
                                //
                            }
                        }
                    }
                }

                
                // Draw
                for(int i = 0; i < 10; i++ ){
                    if(!enemys[i].die){
                        SDL_RenderCopy(renderer, enemys[i].texture, NULL, &enemys[i].rect);
                    }
                }

                for(int i = 0; i < 10; i++ ){
                    if(!fires[i].die){
                        SDL_RenderCopy(renderer, fires[i].texture, NULL, &fires[i].rect);
                    }
                }

                SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);

                // Draw score
                SDL_RenderCopy(renderer, score_texture, NULL, &score_rect);

                if(gameover){
                    SDL_RenderCopy(renderer, gameover_texture, NULL, &gameover_rect);
                }

                // Update screen
                SDL_RenderPresent(renderer);

                frameTime = SDL_GetTicks() - frameStart;
                if(frameDelay > frameTime){
                    SDL_Delay(frameDelay - frameTime);
                }
            }

            endAudio();

            // Quit SDL_image
            IMG_Quit();

            // Quit SDL_ttf
            TTF_CloseFont(font);
            TTF_Quit();

            // Destroy renderer
            SDL_DestroyRenderer(renderer);

        }

        // Destroy window
        SDL_DestroyWindow(window);
    }

    // Quit SDL
    SDL_Quit();
    return 0;
}
