#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <SDL3/SDL.h>

struct GLWindowContext {
    SDL_Window* window;
    SDL_GLContext context;
    const char* glsl_version;
    float scale;
};

GLWindowContext InitGLWindow(const char* title, int width, int height);

#endif
