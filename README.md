# fps
model -> res/models/pistol/scene.gltf

# dependancies
vcpkg install sdl2:x64-windows  
vcpkg install glm:x64-windows

# basic run-down
Camera.cpp - handles the camera  
Main.cpp - contains main while loop, sets up the model, map, viewport etc  
Map.cpp - creates and draws grid in the level  
ModelLoader.cpp - contains the initial loading of the model (tinygltf::model)  
Shader.cpp - contains basic shader loading/using system  
Utils/GLMUtils.cpp - contains to_string capability for glm::vec2, glm::vec3, glm::vec4, glm::quat (for debugging)  
Window/WindowManager.cpp - handles the main SDL2 window, fps counter, and inputs

# inputs
WASD - move forward, left, down, right  
EQ - move up and down (y axis)  
SHIFT - draw models in wireframe mode  
CTRL - increased speed when held (affects WASD & EQ)