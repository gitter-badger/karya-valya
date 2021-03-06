#pragma once

#include "ApproxGLImage.h"
#include "GLSprite.h"

class Screen
{
public:
    Screen(unsigned int x, unsigned int y, bool fullscreen = false);
    void Draw(const GLSprite* sprite, int x, int y, int imageW, int imageH, float angle = 0.0f);
    void Draw(const ApproxGLImage* sprite, int x_ul, int y_ul, int x_dr, int y_dr);
    void ResetScreen(int x, int y, int bpp, Uint32 flags);
    void Clear();
    void Swap();
    bool Fail();

    void PerformSizeUpdate();

    int w();
    int h();
private:
    bool fail_;
    SDL_Surface* screen_;
};

bool IsScreenValid();
Screen& GetScreen();
void SetScreen(Screen* scr);
