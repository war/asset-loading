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
	
public:
	Mesh(Camera* cam, ModelLoader* model_loader, Shader* _shader, WindowManager* win_manager);
	~Mesh();                                    
	
	void update();
};





#endif //MESH_H
