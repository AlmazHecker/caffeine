#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GLFW/glfw3.h>

struct GLWindowContext {
    GLFWwindow* window;
    void* context; // Not used with GLFW, placeholder for compatibility
    const char* glsl_version;
    float scale;
};

GLWindowContext InitGLWindow(const char* title, int width, int height);

#endif
