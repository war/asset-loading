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
	MeshDataStruct* mesh_data;
	Shader* shader;
	WindowManager* window_manager;
	DirectionalLight* direct_light;
	
	VAO vao {};
	VBO vbo {};
	EBO ebo {};
	
	std::vector<GLfloat> vertex_data_array;//holds all of vertex pos, normal uv, joints, etc
	std::vector<GLuint> vertex_indices_array;//vertex indices used in the EBO for more optimized rendering
	
	glm::vec3 position = glm::vec3(0.f);
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 scale = glm::vec3(1.f);
	
	glm::mat4 modelMatrix = glm::mat4(1.f);
	
	int current_animation_frame {};
	float current_animation_time {};
	float playback_speed = 1.f;
	
	std::vector<glm::mat4> bone_transform_matrix_array;//stores bone animated transform [pos/rot/scale] for EACH BONE.
	std::vector<glm::mat4> bone_skinned_matrix_array;//store the FINAL skinned matrix which will be sent directly to shader
	
	bool enable_backface_culling = true;
	
	GLenum render_mode = GL_TRIANGLES;
	
public:
	Mesh(Camera* cam, ModelLoader* model_loader, MeshDataStruct* _mesh_data, Shader* _shader, WindowManager* win_manager, DirectionalLight* _direct_light);
	~Mesh();                                    
	
	void update();
	void updateAnimation();
	void updateSkinnedAnimation();
	
	void setModelMatrix(const glm::mat4& mat) {	modelMatrix = mat; mesh_data->modelMatrix = mat;	}
	void setTranslation(const glm::vec3& pos) {	position = pos; mesh_data->translation = pos;	}
	void setRotation(const glm::quat& rot) {	rotation = rot; mesh_data->rotation = rot;	}
	void setScale(const glm::vec3& _scale) {	scale = _scale; mesh_data->scale = _scale;	}
	
	glm::mat4 getModelMatrix() const	{ return mesh_data->modelMatrix;		}
	glm::vec3 getTranslation() const {	return mesh_data->translation;	}
	glm::quat getRotation() const {	return mesh_data->rotation;	}
	glm::vec3 getScale() const {	return mesh_data->scale;	}
	
	MeshDataStruct* getMeshData() const	{	return mesh_data; }
	
	glm::vec3 calculateCurrentTranslation(const AnimationDataStruct& animation_data);
	glm::quat calculateCurrentRotation(const AnimationDataStruct& animation_data);
	glm::vec3 calculateCurrentScale(const AnimationDataStruct& animation_data);
	
	inline void enableBackFaceCulling(bool mode) {	enable_backface_culling = mode;	}
	inline void setRenderingMode(GLenum mode)	{	render_mode = mode;	}
};





#endif //MESH_H
