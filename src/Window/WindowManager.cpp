#include "WindowManager.h"

WindowManager::WindowManager(
    const std::string& title, int width, int height) : 
        window(nullptr), 
        glContext(nullptr), 
        quit(false), 
        width(width), 
        height(height),
        aspectRatio(width/height),
        title(title), 
        isShiftPressed(false),
        firstMouse(true),
        lastX(0),
        lastY(0){}

WindowManager::~WindowManager() {
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
}

bool WindowManager::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

void WindowManager::handleEvents() {
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
    } 
}

void WindowManager::clear() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WindowManager::swapBuffers() {
    SDL_GL_SwapWindow(window);
}

bool WindowManager::shouldClose() const {
    return quit;
}

void WindowManager::updateTitle(std::string title) {
    SDL_SetWindowTitle(window, title.c_str());
}

void WindowManager::updateFPS() {
    currentTime = SDL_GetTicks();
    frameCount++;

    deltaTime = currentTime - lastTime;

    if (deltaTime >= updateEvery * 1000) {
        fps = frameCount * 1000.0f / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;

        std::string title = std::format("FPS - FPS: {:.2f}", fps);

        updateTitle(title);
    }
}

void WindowManager::processInput(Camera& camera, float deltaTime)
{
		sdlCurrTime = SDL_GetTicks();
	
    float sDeltaTime = deltaTime / 1000;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            quit = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                quit = true;
            else if (event.key.keysym.sym == SDLK_p) {
                mouseCaptured = !mouseCaptured;

                if (mouseCaptured) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    SDL_ShowCursor(SDL_DISABLE);
                }
                else {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    SDL_ShowCursor(SDL_TRUE);
                }
            }
        }
        else if (event.type == SDL_MOUSEMOTION && mouseCaptured)
        {
            float xoffset = event.motion.xrel;
            float yoffset = -event.motion.yrel; // Reversed since y-coordinates go from bottom to top

            camera.ProcessMouseMovement(xoffset, yoffset);
        }
        else if (event.type == SDL_MOUSEWHEEL && mouseCaptured)
        {
            camera.ProcessMouseScroll(static_cast<float>(event.wheel.y));
        }
    }

    if (mouseCaptured) {
        const Uint8* state = SDL_GetKeyboardState(NULL);

        if (state[SDL_SCANCODE_LCTRL]) {
            camera.MovementSpeed = SPEED_FAST;
        }
        else {
            camera.MovementSpeed = SPEED_SLOW;
        }

        if (state[SDL_SCANCODE_W])
            camera.ProcessKeyboard(FORWARD, sDeltaTime);
        if (state[SDL_SCANCODE_S])
            camera.ProcessKeyboard(BACKWARD, sDeltaTime);
        if (state[SDL_SCANCODE_A])
            camera.ProcessKeyboard(LEFT, sDeltaTime);
        if (state[SDL_SCANCODE_D])
            camera.ProcessKeyboard(RIGHT, sDeltaTime);
        if (state[SDL_SCANCODE_E])
            camera.ProcessKeyboard(UP, sDeltaTime);
        if (state[SDL_SCANCODE_Q])
            camera.ProcessKeyboard(DOWN, sDeltaTime);

        ShowWireframe(state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]);
    }
}

void WindowManager::ShowWireframe(bool isShiftPressed) {
    if (isShiftPressed) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void WindowManager::SetVSyncMode(bool mode) const{
	SDL_GL_SetSwapInterval(static_cast<short>(mode));
}

void WindowManager::updateDeltaTime(){
	m_deltaTime = (float)(sdlCurrTime - sdlLastTime)/1000.f;
	
	sdlLastTime = sdlCurrTime;
	
	std::cout << m_deltaTime << std::endl;
}
