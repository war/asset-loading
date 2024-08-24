//Mehdi Msayib - glTF asset loader - Mesh file for construction and rendering of loaded models

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
#include "Utils.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

class Mesh{
public:
	Camera* camera;
	ModelLoader* model;
	MeshDataStruct mesh_data;
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
	float playback_speed = 1.f;
	
	std::vector<glm::mat4> bone_transform_matrix_array;//stores bone animated transform [pos/rot/scale] for EACH BONE.
	std::vector<glm::mat4> bone_skinned_matrix_array;//store the FINAL skinned matrix which will be sent directly to shader
	
	
public:
	Mesh(Camera* cam, ModelLoader* model_loader, MeshDataStruct _mesh_data, Shader* _shader, WindowManager* win_manager);
	~Mesh();                                    
	
	void update();
	void updateAnimation();
	void updateSkinnedAnimation();
	
	void setModelMatrix(const glm::mat4& mat) {	modelMatrix = mat;	}
	void setTranslation(const glm::vec3& pos) {	position = pos;	}
	void setRotation(const glm::quat& rot) {	rotation = rot;	}
	void setScale(const glm::vec3& _scale) {	scale = _scale;	}
	
	glm::mat4 getModelMatrix() const	{ return modelMatrix;		}
	glm::vec3 getTranslation() const {	return position;	}
	glm::quat getRotation() const {	return rotation;	}
	glm::vec3 getScale() const {	return scale;	}
	
	MeshDataStruct getMeshData()	{	return mesh_data; }
	
	glm::vec3 calculateCurrentTranslation(const AnimationDataStruct& animation_data);
	glm::quat calculateCurrentRotation(const AnimationDataStruct& animation_data);
	glm::vec3 calculateCurrentScale(const AnimationDataStruct& animation_data);
};





#endif //MESH_H
