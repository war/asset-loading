//Mehdi Msayib - glTF asset loader - Utils file which houses commonly used functions and structs 

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>

#include "../external/tinygltf/tiny_gltf.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>

#include <glad/glad.h>

enum TextureType{
	DIFFUSE = 0,
	NORMAL = 1,
	METAL = 2,
	SPECULAR = 3,
	ROUGHNESS = 4
};

struct TextureDataStruct{
	TextureType type;
	GLuint tex_id {};
};

struct AnimationDataStruct{
	std::string name;
	
	int node_index = -1;
	tinygltf::Node node;
	
	bool has_root = false;
	int root_idx = -1;
	
	bool has_animation = false;
	
	glm::vec3 translation = glm::vec3(0.f);//relative to parent bone
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);//relative to parent bone
	glm::vec3 scale = glm::vec3(1.f);//relative to parent bone
	
	bool is_mesh = false;
	bool is_empty = false;
	bool is_bone = false;
	
	float current_animation_time {};
	int current_animation_frame {};
	float playback_speed = 1.f;
	
	std::vector<int> child_array;
	
	std::vector<float> time_array;//largest time array
	std::vector<float> trans_time_array;
	std::vector<float> rot_time_array;
	std::vector<float> scale_time_array;
	std::vector<glm::vec3> translation_anim_array;
	std::vector<glm::quat> rotation_anim_array;
	std::vector<glm::vec3> scale_anim_array;
};

struct MaterialDataStruct{
	bool has_material = false;
	std::string name;
	glm::vec4 base_color = glm::vec4(0.f);
	float metalness = 0.f;
	float roughness = 0.f;
};

struct MeshDataStruct{
	int node_index = -1;//index in the node array
	
	std::string name;
	
	//global transforms
	glm::vec3 translation = glm::vec3(0.f);
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 scale = glm::vec3(1.f);
	
	glm::mat4 modelMatrix = glm::mat4(1.f);
	glm::mat4 matrix_transform = glm::mat4(1.f);//base matrix loaded in from gltf [if it exists]
	bool has_matrix_transform = false;
	
	//vertex norm/uv etc data
	std::vector<glm::vec3> vertex_positions_array;
	std::vector<glm::vec3> vertex_normals_array;
	std::vector<glm::vec2> vertex_uvs_array;
	std::vector<unsigned int> vertex_indices_array;
	
	//textures
	std::map<TextureType, TextureDataStruct> texture_map;
	
	//materials
	MaterialDataStruct material_data;
	
	//animation data FOR THIS MESH
	AnimationDataStruct animation_data {};
	bool inherits_animation = false;
	
	//skinning data
	bool has_skin = false;
	tinygltf::Skin skin;
	std::vector<glm::vec4> joints_array;
	std::vector<glm::vec4> weights_array;
	std::vector<glm::mat4> inverse_bind_matrix_array;
	
	//root and sub-node
	bool has_childs = false;
	std::vector<int> childs_array;
};

struct EmptyNode{
	int node_index = -1;
	tinygltf::Node node;
	
	std::string name;
	
	glm::vec3 translation = glm::vec3(0.f);
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 scale = glm::vec3(1.f);
	
	glm::mat4 modelMatrix = glm::mat4(1.f);
	glm::mat4 matrix_transform = glm::mat4(1.f);//base matrix loaded in from gltf [if it exists]
	bool has_matrix_transform = false;
	
	bool has_childs = false;
	std::vector<int> child_array;
	
	bool has_parent = false;
	int parent_idx = -1;
	
	bool is_root = false;
	
	bool has_animation = false;
	std::string animation_name;
	bool inherits_animation = false;
	
	tinygltf::Animation anim_gltf;
	
	AnimationDataStruct animation_data;
	
	bool has_root = false;
	int root_idx = -1;
	
	void setModelMatrix(const glm::mat4& mat) {	modelMatrix = mat;	}
	void setTranslation(const glm::vec3& pos) {	translation = pos; animation_data.translation = pos;	}
	void setRotation(const glm::quat& rot) {	rotation = rot; animation_data.rotation = rot;	}
	void setScale(const glm::vec3& _scale) {	scale = _scale; animation_data.scale = _scale;	}
	
	glm::mat4 getModelMatrix() const	{ return modelMatrix;		}
	glm::vec3 getTranslation() const {	return translation;	}
	glm::quat getRotation() const {	return rotation;	}
	glm::vec3 getScale() const {	return scale;	}
	
};

struct DirectionalLight{
	float strength = 1.f;
	float specular = 1.f;
	glm::vec3 direction = glm::vec3(0.f, -1.f, 0.f);
	glm::vec3 color = glm::vec3(1.f);
};

//basic print
inline void PRINT(const std::string& message){
	std::cout << message << std::endl;
}

//warning
inline void PRINT_WARN(const std::string& message){
	PRINT("\x1B[38;2;255;120;10mWARNING - \x1B[0m" + message);
}

//print custom color
inline void PRINT_COLOR(const std::string& message, short red, short green, short blue){
	PRINT( std::string("\x1B[38;2;") + std::to_string(red) + ";" + std::to_string(green) + ";" + std::to_string(blue) + "mLOG - \x1B[0m" + message);
}

inline void printGlmVec3(const glm::vec3& v){
	std::cout << "[x: " << v.x << ", y: " << v.y << ", z: " << v.z << "]" << std::endl;
}

inline void printGlmVec4(const glm::vec4& v){
	std::cout << "[x: " << v.x << ", y: " << v.y << ", z: " << v.z << ", w: " << v.w << "]" << std::endl;
}

inline void printGlmMat4(const glm::mat4& m){
	std::cout << "Mat4: " << std::endl;
	std::cout << "[x: " << m[0].x << ", y: " << m[0].y << ", z: " << m[0].z << ", w:" << m[0].w << "]" << std::endl;
	std::cout << "[x: " << m[1].x << ", y: " << m[1].y << ", z: " << m[1].z << ", w:" << m[1].w << "]" << std::endl;
	std::cout << "[x: " << m[2].x << ", y: " << m[2].y << ", z: " << m[2].z << ", w:" << m[2].w << "]" << std::endl;
	std::cout << "[x: " << m[3].x << ", y: " << m[3].y << ", z: " << m[3].z << ", w:" << m[3].w << "]" << std::endl;
	
}

inline void printGlmQuat(const glm::quat& q){
	std::cout << "[x: " << q.x << ", y: " << q.y << ", z: " << q.z << ", w:" << q.w << "]" << std::endl;
}

inline glm::mat4 createTRSmatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale){
	//apply translation
	glm::mat4 mat = glm::translate(glm::mat4(1.f), position);
	
	//apply rotation
	mat = mat * glm::mat4(rotation);
	
	//finally apply scale
	mat = glm::scale(mat, scale);
	
	return mat;
}

inline constexpr float integerFromFloat(float f, int decimal_count){
	return (int)(f*std::pow(10, decimal_count));
}

inline void __GL_ERROR_THROW__(const std::string& optional_string = ""){
	GLenum error_check = glGetError();
	
	if(error_check != GL_NO_ERROR){
		std::string core_message = "**** [ERROR] -- ";
		
		if(error_check == GL_INVALID_ENUM)
			core_message += optional_string + " (code: GL_INVALID_ENUM)";
		
		if(error_check == GL_INVALID_VALUE)
			core_message += optional_string + " (code: GL_INVALID_VALUE)";
		
		if(error_check == GL_INVALID_OPERATION)
			core_message += optional_string + " (code: GL_INVALID_OPERATION)";
		
		if(error_check == GL_INVALID_FRAMEBUFFER_OPERATION)
			core_message += optional_string + " (code: GL_INVALID_FRAMEBUFFER_OPERATION)";
		
		if(error_check == GL_OUT_OF_MEMORY)
			core_message += optional_string + " (code: GL_OUT_OF_MEMORY)";
		
		throw std::logic_error(core_message + " ****");
	}
}

#endif //UTILS_H
