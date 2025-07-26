

#include "gl_context.h"
#include <GLFW/glfw3.h>

GLWindowContext InitGLWindow(const char* title, int width, int height) {
    if (!glfwInit()) {
        return {};
        ; // handle error
    }

    const char* glsl_version;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);


    GLFWwindow* win = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!win) {
        // error handling here
        return {}; // or throw
    }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    float scale_x, scale_y;
    glfwGetWindowContentScale(win, &scale_x, &scale_y);
    float scale = (scale_x + scale_y) * 0.5f;

    return { win, nullptr, glsl_version, scale };
}
