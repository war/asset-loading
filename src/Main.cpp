#include "Main.h"

#include <iostream>

#include "Window/WindowManager.h"
#include "Shader.h"
#include "Camera.h"
#include "Map.h"
#include "ModelLoader.h"

int main(int argc, char* argv[]) {
    //Window
    WindowManager windowManager("FPS", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!windowManager.init()) {
        return -1;
    }

    //Camera
    Camera camera(glm::vec3(0.0f, 5.0f, 3.0f));

    //Shaders
    Shader gridShader("grid.vert", "grid.frag");
    if (gridShader.isValid() == 0) {
        std::cout << "Failed to load grid shaders." << std::endl;
        return -1;
    }

    //Scene
    Map gridMap(40, 2.f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::string windowTitle = "";

    SDL_GL_SetSwapInterval(0);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    //exit(0);

    ModelLoader model;
    model.LoadModel("res/models/pistol/scene.gltf");

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor = glm::vec3(0.5f, 0.5f, 0.5f);
    
    float aspectRatio = windowManager.getAspectRatio();

    while (!windowManager.shouldClose()) {
        windowManager.clear();

        msCurrentTime = SDL_GetTicks();
        msDeltaTime = msCurrentTime - msLastTime;
        msLastTime = msCurrentTime;

        windowManager.processInput(camera, msDeltaTime);

        glm::mat4 projection = glm::perspective(camera.FovRads, aspectRatio, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        gridShader.use();
        gridShader.setMat4("projection", projection);
        gridShader.setMat4("view", view);
        gridMap.draw(gridShader);

        glClear(GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        windowManager.swapBuffers();
        windowManager.updateFPS();
    }

    return 0;
}