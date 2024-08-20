#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Window/WindowManager.h"
#include "ErrorLogger.h"
#include "ModelLoader.h"
#include "Camera.h"
#include "Shader.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

class Mesh{
private:
	Camera* camera;
	ModelLoader* model;
	Shader* shader;
	WindowManager* window_manager;
	
	VAO vao {};
	VBO vbo {};
	EBO ebo {};
	
	std::vector<GLfloat> tri_vertices;
	std::vector<GLuint> tri_indices;
	
	glm::vec3 position = glm::vec3(0.f);
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 scale = glm::vec3(1.f);
	
	glm::mat4 modelMatrix = glm::mat4(1.f);
	
	int current_animation_frame {};
	float current_animation_time {};
	
public:
	Mesh(Camera* cam, ModelLoader* model_loader, Shader* _shader, WindowManager* win_manager);
	~Mesh();                                    
	
	void update();
	void updateAnimation();
	
	glm::vec3 calculateCurrentTranslation(const AnimationDataStruct& animation_data);
	glm::quat calculateCurrentRotation(const AnimationDataStruct& animation_data);
	glm::vec3 calculateCurrentScale(const AnimationDataStruct& animation_data);
};





#endif //MESH_H
