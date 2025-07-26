// Caffeine App: Prevents PC from sleeping with beautiful animations
// Based on Dear ImGui + SDL3 + OpenGL

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include "imgui-toggle/imgui_toggle.h"
#include "gl_context/gl_context.h"
#include <SDL3/SDL_opengl.h>
#include <cmath>

// Animation state
struct AnimationState {
    float fadeAlpha = 0.0f;
    float pulseScale = 1.0f;
    float glowIntensity = 0.0f;
    float messageTimer = 0.0f;
    bool isAnimating = false;
    bool lastToggleState = false;
};

// Easing function for smooth animations
float easeInOutQuad(float t) {
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

// Draw animated coffee cup icon
void DrawCoffeeIcon(ImVec2 pos, float size, float alpha, bool isActive) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Coffee cup body
    ImU32 cupColor = isActive ?
        IM_COL32(139, 69, 19, (int)(255 * alpha)) :  // Brown when active
        IM_COL32(100, 100, 100, (int)(255 * alpha)); // Gray when inactive

    draw_list->AddRectFilled(
        ImVec2(pos.x, pos.y + size * 0.3f),
        ImVec2(pos.x + size * 0.6f, pos.y + size),
        cupColor, size * 0.1f
    );

    // Coffee cup handle
    draw_list->AddCircle(
        ImVec2(pos.x + size * 0.7f, pos.y + size * 0.6f),
        size * 0.15f, cupColor, 12, size * 0.03f
    );

    // Steam (only when active)
    if (isActive) {
        float steamAlpha = alpha * 0.7f;
        ImU32 steamColor = IM_COL32(255, 255, 255, (int)(255 * steamAlpha));

        float time = ImGui::GetTime();
        float sharedPhase = time * 2.5f; // Shared phase for synchronization

        // Steam consists of 3 vertical wavy lines rising above cup
        for (int i = 0; i < 3; i++) {
            float x = pos.x + size * 0.2f + i * size * 0.12f;
            float baseY = pos.y + size * 0.2f;  // Lowered by 0.1 * size
            float height = size * 0.4f;

            const int segments = 20;
            for (int seg = 0; seg < segments; seg++) {
                float t0 = (float)seg / segments;
                float t1 = (float)(seg + 1) / segments;

                // Synchronized wave using sharedPhase, phase shifted by line index for slight delay
                float wave0 = sin(sharedPhase + i * 0.5f + t0 * 3.14f * 2) * size * 0.02f; // increased amplitude
                float wave1 = sin(sharedPhase + i * 0.5f + t1 * 3.14f * 2) * size * 0.02f;

                ImVec2 p0 = ImVec2(x + wave0, baseY - height * t0);
                ImVec2 p1 = ImVec2(x + wave1, baseY - height * t1);

                float alphaSegment = steamAlpha * (1.0f - t1);

                draw_list->AddLine(p0, p1, IM_COL32(255, 255, 255, (int)(255 * alphaSegment)), size * 0.03f); // thicker line
            }
        }
    }


}

// Draw sleeping "Z" particles
void DrawSleepParticles(ImVec2 center, float size, float alpha) {
    if (alpha <= 0.0f) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 zColor = IM_COL32(150, 150, 255, (int)(255 * alpha));

    float time = ImGui::GetTime();

    // Multiple floating Z's
    for (int i = 0; i < 5; i++) {
        float phase = i * 0.8f;
        float floatOffset = sin(time * 2.0f + phase) * size * 0.3f;
        float scaleVariation = 0.7f + 0.3f * sin(time * 1.5f + phase);

        ImVec2 zPos = ImVec2(
            center.x + cos(phase) * size * 0.8f,
            center.y - size * 0.5f + floatOffset - i * size * 0.2f
        );

        float zSize = size * 0.15f * scaleVariation;

        // Draw "Z" shape
        ImVec2 p1 = ImVec2(zPos.x - zSize, zPos.y - zSize);
        ImVec2 p2 = ImVec2(zPos.x + zSize, zPos.y - zSize);
        ImVec2 p3 = ImVec2(zPos.x - zSize, zPos.y + zSize);
        ImVec2 p4 = ImVec2(zPos.x + zSize, zPos.y + zSize);

        draw_list->AddLine(p1, p2, zColor, zSize * 0.2f);
        draw_list->AddLine(p2, p3, zColor, zSize * 0.2f);
        draw_list->AddLine(p3, p4, zColor, zSize * 0.2f);
    }
}

int main(int, char**)
{
    // Setup SDL and OpenGL
    GLWindowContext ctx = InitGLWindow("Caffeine - Keep PC Awake", 400, 300);

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(ctx.scale);
    style.FontScaleDpi = ctx.scale;

    // Customize colors for caffeine theme
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.1f, 0.05f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.15f, 0.08f, 1.0f);

    // Setup backends
    ImGui_ImplSDL3_InitForOpenGL(ctx.window, ctx.context);
    ImGui_ImplOpenGL3_Init(ctx.glsl_version);

    // App state
    bool keepAwake = false;
    AnimationState anim;

    // Main loop
    bool done = false;
    while (!done)
    {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(ctx.window))
                done = true;
        }

        if (SDL_GetWindowFlags(ctx.window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Check for toggle state change
        if (keepAwake != anim.lastToggleState) {
            anim.isAnimating = true;
            anim.messageTimer = 0.0f;
            anim.lastToggleState = keepAwake;
        }

        // Update animations
        float deltaTime = io.DeltaTime;
        if (anim.isAnimating) {
            anim.messageTimer += deltaTime;

            // Fade in message
            if (anim.messageTimer < 1.0f) {
                anim.fadeAlpha = easeInOutQuad(anim.messageTimer);
            }
            else if (anim.messageTimer < 3.0f) {
                anim.fadeAlpha = 1.0f;
            }
            else if (anim.messageTimer < 4.0f) {
                anim.fadeAlpha = 1.0f - easeInOutQuad(anim.messageTimer - 3.0f);
            }
            else {
                anim.isAnimating = false;
                anim.fadeAlpha = 0.0f;
            }
        }

        // Pulse effect for active state
        if (keepAwake) {
            anim.pulseScale = 1.0f + 0.1f * sin(ImGui::GetTime() * 2.0f);
            anim.glowIntensity = 0.5f + 0.3f * sin(ImGui::GetTime() * 1.5f);
        }
        else {
            anim.pulseScale = 1.0f;
            anim.glowIntensity = 0.0f;
        }

        // Main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Caffeine", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        // Center content
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 center = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);

        // Draw coffee icon
        ImVec2 iconPos = ImVec2(center.x - 40, center.y - 100);
        DrawCoffeeIcon(iconPos, 80, 1.0f, keepAwake);

        // Add glow effect when active
        if (keepAwake && anim.glowIntensity > 0) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImU32 glowColor = IM_COL32(255, 200, 100, (int)(50 * anim.glowIntensity));
            draw_list->AddCircleFilled(
                ImVec2(center.x, center.y - 60),
                100 * anim.pulseScale, glowColor
            );
        }

        // Main toggle button
        ImGui::SetCursorPos(ImVec2(center.x - 60, center.y - 20));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 25.0f);

        // Scale the toggle
        if (anim.pulseScale != 1.0f) {
            ImGui::SetWindowFontScale(anim.pulseScale);
        }

        bool toggleChanged = ImGui::Toggle("##caffeine_toggle", &keepAwake,
            ImGuiToggleFlags_Animated, ImVec2(120.0f, 60.0f));

        if (anim.pulseScale != 1.0f) {
            ImGui::SetWindowFontScale(1.0f);
        }

        ImGui::PopStyleVar();

        // Status text
        ImGui::SetCursorPos(ImVec2(center.x - 50, center.y + 60));
        if (keepAwake) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), "PC Staying Awake");
        }
        else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Sleep Allowed");
        }

        // Animated message overlay
        if (anim.isAnimating && anim.fadeAlpha > 0) {
            ImVec2 messagePos = ImVec2(center.x - 100, center.y + 100);
            ImGui::SetCursorPos(messagePos);

            if (keepAwake) {
                // Awake message
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, anim.fadeAlpha));
                ImGui::Text("Your PC won't sleep!");
                ImGui::SetCursorPos(ImVec2(messagePos.x + 10, messagePos.y + 25));
                ImGui::Text("Caffeine is active");
                ImGui::PopStyleColor();
            }
            else {
                // Sleep message
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 1.0f, anim.fadeAlpha));
                ImGui::Text("Going to sleep...");
                ImGui::SetCursorPos(ImVec2(messagePos.x + 20, messagePos.y + 25));
                ImGui::Text("Sweet dreams!");
                ImGui::PopStyleColor();

                // Draw sleep particles
                DrawSleepParticles(center, 100, anim.fadeAlpha);
            }
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

        // Dynamic background color based on state
        ImVec4 bgColor = keepAwake ?
            ImVec4(0.05f, 0.08f, 0.02f, 1.0f) :  // Slight green tint when active
            ImVec4(0.02f, 0.02f, 0.08f, 1.0f);   // Slight blue tint when sleeping

        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(ctx.window);

        // TODO: Implement actual sleep prevention logic here
        // When keepAwake is true, you would call platform-specific APIs to prevent sleep:
        // - Windows: SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED)
        // - macOS: IOPMAssertionCreateWiCreateWithName
        // - Linux: systemd-inhibit or dbus calls
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(ctx.context);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}