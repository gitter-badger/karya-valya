#include "SdlInit.h"

#include <SDL.h>

#include "core/Constheader.h"
#include "Sprite.h"

bool InitSDL()
{
    Uint32 rmask, gmask, bmask, amask;
    SetMasks(&rmask, &gmask, &bmask, &amask);

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    { 
        SYSTEM_STREAM << "Unable to init SDL: " << SDL_GetError() << std::endl; 
        SDL_Delay(10000);
        return false;
    }
    atexit(SDL_Quit);

    return true;
}
