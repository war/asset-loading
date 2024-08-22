#include "Main.h"

#include <iostream>

#include "Window/WindowManager.h"
#include "Shader.h"
#include "Camera.h"
#include "Map.h"
#include "ModelLoader.h"
#include "Mesh.h"

std::vector<Mesh*> mesh_array;

int main(int argc, char* argv[]) {
    //Window
    WindowManager windowManager("FPS", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!windowManager.init()) {
        return -1;
    }
		
    //Camera
    Camera camera(glm::vec3(0.0f, 15.0f, 25.0f));

    //Shaders
    Shader gridShader("grid.vert", "grid.frag");
    if (gridShader.isValid() == 0) {
        std::cout << "Failed to load grid shaders." << std::endl;
        return -1;
    }
		Shader basicShader("basic_shader.vert", "basic_shader.frag");
		if (basicShader.isValid() == 0) {
			std::cout << "Failed to load basic_shader." << std::endl;
			return -1;
		}
		Shader skinnedShader("skinned_shader.vert", "skinned_shader.frag");
		if (skinnedShader.isValid() == 0) {
			std::cout << "Failed to load skinned_shader." << std::endl;
			return -1;
		}

    //Scene
    Map gridMap(40, 2.f);

    std::string windowTitle = "";

		//turn on Vsync
		windowManager.SetVSyncMode(true);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);


		//load in glTF model (meshes, animations, skinning, textures etc)
    ModelLoader* model = new ModelLoader("res/models/pistol/multiple-meshes.gltf", "material_baseColor", "material_normal", "material_metallicRoughness");
//    ModelLoader* model = new ModelLoader("res/models/pistol/skinned-hands.gltf", "material_baseColor", "material_normal", "material_metallicRoughness");

		///////////////
		//mesh loading
		///////////////
		//spawn meshes
		for(MeshDataStruct mesh_data : model->mesh_data_struct_array){
			Mesh* mesh = new Mesh(&camera, model, mesh_data, &basicShader, &windowManager);//delete this once finished to avoid memory leaks
			mesh_array.emplace_back(mesh);
		}
	
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
			
        glm::mat4 projection = glm::perspective(camera.FovRads, aspectRatio, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

			  glEnable(GL_BLEND);
		    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gridShader.use();
        gridShader.setMat4("projection", projection);
        gridShader.setMat4("view", view);
        gridMap.draw(gridShader);
				glDisable(GL_BLEND);
//
//        model.Render();
			
				////////////////
				//render meshes
				////////////////
				{
					for(Mesh* mesh : mesh_array)
						mesh->update();
				}
			
        windowManager.swapBuffers();
        windowManager.updateFPS();
			
				windowManager.updateDeltaTime();
    }

	//delete Mesh* objects
	for(Mesh* mesh : mesh_array)
		delete mesh;
	
	//delete ModelLoader class
	delete model;
	
    return 0;
}
