#pragma once

#include <SDL.h>
#include <glad/glad.h>
#include <string>

#include <iostream>
#include "../Camera.h"

class WindowManager {
public:
    WindowManager(const std::string& title, int width, int height);
    ~WindowManager();

    bool init();
    void clear();
    void handleEvents();
    void swapBuffers();

    bool shouldClose() const;

    void updateTitle(std::string);

    void updateFPS();
    void processInput(Camera& camera, float deltaTime);

    void setShouldClose(bool close) { quit = close; }
    bool isShiftKeyPressed() const { return isShiftPressed; }
    void ShowWireframe(bool isShiftPressed);

    int getWidth() { return width; }
    int getHeight() { return height; }

    float getAspectRatio() { return aspectRatio; }
private:
    int width;
    int height;

    float aspectRatio;

    std::string title;

    SDL_Window* window;
    SDL_GLContext glContext;

    bool quit;
    bool mouseCaptured = true;

    // fps counter
    int frameCount = 0;
    Uint32 lastTime = 0.0;
    Uint32 currentTime = 0.0;
    Uint32 deltaTime = 0.0;
    float updateEvery = 0.5; // per second
    float fps = 0.0f;

    // Mouse variables
    float lastX;
    float lastY;
    bool firstMouse;

    bool isShiftPressed;
};
