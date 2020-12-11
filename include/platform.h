#ifndef Platform_H
#define Platform_H

#include <SDL2/SDL.h>

class Platform
{
private:
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};

public:
    Platform(char const* title,
        int windowWidth,
        int windowHeight,
        int textureWidth,
        int textureHeight);

    ~Platform();

    bool ProcessInput(uint8_t* keys);
};

#endif