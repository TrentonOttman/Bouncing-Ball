#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

// 100 Pixel = 1 Meter
double GRAVITY = 9.81 * 100;
double COEFFICIENT_OF_FRICTION = 0.1;
// Ball retains x% of velocity on bounce
double FLOOR_DAMPING_CONSTANT = .8;
double WALL_DAMPING_CONSTANT =.8;
// Ball radius in pixels
int B_RADIUS = 25;

struct Ball {
    double posX;
    double posY;
    double velocityX;
    double velocityY;
    double energyX;
    double energyY;
    double initialEnergyX;
    double initialEnergyY;
    int radius;
    bool atRestX;
    bool atRestY;
};

int main() {

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Initialize random number generator
    std::srand(std::time(nullptr));

    // Initialize mouse coordinates
    int mouseX;
    int mouseY;

    // Find display mode and calculate relevant sizes
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    int screenWidth = dm.w;
    int screenHeight = dm.h;
    int windowWidth = screenWidth*.8;
    int windowHeight = screenHeight*.8;

    // Create the window
    SDL_Window* window = SDL_CreateWindow("SDL2", (screenWidth-windowWidth)/2, (screenHeight-windowHeight)/2, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);

    // Create the renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    bool running = true;
    SDL_Event event;

    std::vector<Ball> balls;

    Uint32 currentTime = 0;
    Uint32 previousTime = SDL_GetTicks();
    while (running) {
        currentTime = SDL_GetTicks();
        float dt = (currentTime-previousTime)/1000.0f;
        if (dt == 0) dt = 0.0001;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
            // On click, create a ball with random X velocity
            if (event.type == SDL_MOUSEBUTTONUP) {
                SDL_GetMouseState(&mouseX, &mouseY);
                // Generate random X velocity in range [-100, 100]
                double randomXVelo = (std::rand() % 2001) - 1000;
                balls.push_back(Ball{double(mouseX), double(mouseY), randomXVelo, 0.0, -1, -1, -1, -1, B_RADIUS, false, false});
                filledCircleRGBA(renderer, mouseX, mouseY, B_RADIUS, 255, 0, 0, 255);
            }
        }

        // Update velocities of balls
        for (int i = 0; i < balls.size(); i++) {
            // Gravity for velocityY
            if (balls[i].atRestY == false) {
                balls[i].velocityY += GRAVITY * dt;
            }
            // Friction for velocityX
            if (balls[i].atRestY == true && balls[i].atRestX == false) {
                if (balls[i].velocityX > 0) {
                    balls[i].velocityX -= abs(COEFFICIENT_OF_FRICTION * GRAVITY * dt);
                }
                if (balls[i].velocityX < 0) {
                    balls[i].velocityX += abs(COEFFICIENT_OF_FRICTION * GRAVITY * dt);
                }
            }
        }

        // Update positions of balls
        for (int i = 0; i < balls.size(); i++) {
            balls[i].posY += balls[i].velocityY * dt;
            balls[i].posX += balls[i].velocityX * dt;
            // If the ball hits the bottom of the screen, move it up and reverse velocity
            if (balls[i].posY >= windowHeight-B_RADIUS) {
                balls[i].posY = windowHeight-B_RADIUS;
                balls[i].velocityY = -(balls[i].velocityY * FLOOR_DAMPING_CONSTANT);
// APPLY FRICTION HERE ALSO MAYBE IDK??
            }
            // If the ball hits the left side of the screen, move it right and reverse velocity
            if (balls[i].posX <= B_RADIUS) {
                balls[i].posX = B_RADIUS;
                balls[i].velocityX = -(balls[i].velocityX * WALL_DAMPING_CONSTANT);
            }
            // If the ball hits the right side of the screen, move it left and reverse velocity
            if (balls[i].posX >= windowWidth-B_RADIUS) {
                balls[i].posX = windowWidth-B_RADIUS;
                balls[i].velocityX = -(balls[i].velocityX * WALL_DAMPING_CONSTANT);  
            }
        }

        // Update energy of balls
        for (int i = 0; i < balls.size(); i++) {
            balls[i].energyX = (0.5 * balls[i].velocityX * balls[i].velocityX);
            balls[i].energyY = (0.5 * balls[i].velocityY * balls[i].velocityY) + (GRAVITY * (windowHeight-(balls[i].posY + B_RADIUS)));
            if (balls[i].initialEnergyX == -1) {
                balls[i].initialEnergyX = balls[i].energyX;
            }
            if (balls[i].initialEnergyY == -1) {
                balls[i].initialEnergyY = balls[i].energyY;
            }
            // If energy is less than .01% of intitial set velocity to 0 and set atRest to true
            if ((balls[i].energyY/balls[i].initialEnergyY)*100 <= 0.01) {
                balls[i].velocityY = 0;
                balls[i].atRestY = true;
            }
            if ((balls[i].energyX/balls[i].initialEnergyX)*100 <= 0.01) {
                balls[i].velocityX = 0;
                balls[i].atRestX = true;
            }
            std::cout << "ENERGY Y: " << balls[i].energyY << std::endl;
            std::cout << "ENERGY Y IS AT " << (balls[i].energyY/balls[i].initialEnergyY)*100 << "% OF INITIAL" << std::endl;
            std::cout << "ENERGY X: " << balls[i].energyX << std::endl;
            std::cout << "ENERGY X IS AT " << (balls[i].energyX/balls[i].initialEnergyX)*100 << "% OF INITIAL" << std::endl;
            std::cout << "AT REST Y IS " << balls[i].atRestY << std::endl;
            std::cout << "AT REST X IS " << balls[i].atRestX << std::endl;
        }

        // Clear Screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Redraw Balls
        for (int i = 0; i < balls.size(); i++) {
            filledCircleRGBA(renderer, balls[i].posX, balls[i].posY, B_RADIUS, 255, 0, 0, 255);
        }

        SDL_RenderPresent(renderer);
        previousTime = currentTime;
    }

    std::cout << "There are " << size(balls) << " balls!\n";

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}