#include "Main.h"

#include <iostream>

#include "Window/WindowManager.h"
#include "AnimationPlayer.h"
#include "ModelLoader.h"
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Map.h"

std::vector<Mesh*> mesh_array;//stores all meshes

int main(int argc, char* argv[]) {
    //Window
    WindowManager windowManager("FPS", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!windowManager.init()) {
        return -1;
    }
		
    //Camera
    Camera camera(glm::vec3(0.f, 69.6417f, 103.036f));

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
		Shader defaultShader("default_shader.vert", "default_shader.frag");
		if (defaultShader.isValid() == 0) {
			std::cout << "Failed to load default_shader." << std::endl;
			return -1;
		}

    //Scene
    Map gridMap(40, 2.f);

    std::string windowTitle = "";

		//turn on Vsync
		windowManager.SetVSyncMode(true);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

		//add directional light
		DirectionalLight* direct_light = new DirectionalLight;
	
		//load in glTF model (meshes, animations, skinning, textures etc)
    ModelLoader* model = new ModelLoader("res/models/pistol/scene.gltf");
//    ModelLoader* model = new ModelLoader("res/models/m16/scene.gltf");

		///////////////
		//mesh loading
		///////////////
		//spawn meshes
		for(MeshDataStruct mesh_data : model->getMeshDataArray()){
			Mesh* mesh = new Mesh(&camera, model, mesh_data, &defaultShader, &windowManager, direct_light);//delete Mesh* once finished to avoid memory leaks
			mesh_array.emplace_back(mesh);
		}
	
		//add AnimationPlayer system
		AnimationPlayer animation_player(model, &mesh_array, &windowManager);
	
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

//    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
//    glm::vec3 objectColor = glm::vec3(0.5f, 0.5f, 0.5f);
    
    float aspectRatio = windowManager.getAspectRatio();
	
    while (!windowManager.shouldClose()) {
        windowManager.clear();

        msCurrentTime = SDL_GetTicks();
        msDeltaTime = msCurrentTime - msLastTime;
        msLastTime = msCurrentTime;

        windowManager.processInput(camera, msDeltaTime);

				//must clear depth buffer as well
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
        glm::mat4 projection = glm::perspective(camera.FovRads, aspectRatio, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

				//render grid (if LShift pressed)
				if(windowManager.isLShiftPressed())
				{
					glDisable(GL_CULL_FACE);
				  glEnable(GL_BLEND);
			    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	        gridShader.use();
	        gridShader.setMat4("projection", projection);
	        gridShader.setMat4("view", view);
	        gridMap.draw(gridShader);
					glDisable(GL_BLEND);
				}
			
				//////////////////////
				//render glTF meshes
				//////////////////////
				for(Mesh* mesh : mesh_array){
					
					//[optional] change render mode (triangles, point cloud, lines) if K or L pressed
					if( windowManager.isKkeyPressed() )
						mesh->setRenderingMode(GL_POINTS);
					else if( windowManager.isLkeyPressed() )
						mesh->setRenderingMode(GL_LINES);
					else
						mesh->setRenderingMode(GL_TRIANGLES);
					
					mesh->update();
				}
			
				//update animation system
				animation_player.update();
			
				//reset animations if R pressed
				if(windowManager.isRKeyPressed())
					animation_player.resetAnimations();
				
        windowManager.swapBuffers();
        windowManager.updateFPS();
			
				windowManager.updateDeltaTime();
    }

		//delete Mesh* objects
		for(Mesh* mesh : mesh_array)
			delete mesh;
		
		//delete ModelLoader class
		delete model;
	
		//delete directional light
		delete direct_light;
	
    return 0;
}
