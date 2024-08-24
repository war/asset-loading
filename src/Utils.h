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

struct AnimationDataStruct{
	std::string name;
	
	int node_index = -1;
	
	bool has_root = false;
	int root_idx = -1;
	
	bool has_animation = false;
	
	bool is_mesh = false;
	bool is_empty = false;
	bool is_bone = false;
	
	float current_animation_time {};
	float playback_speed = 1.f;
	
	std::vector<int> child_array;
	
	std::vector<float> time_array;//should be a time for each of trans/rot/scale
	std::vector<glm::vec3> translation_anim_array;
	std::vector<glm::quat> rotation_anim_array;
	std::vector<glm::vec3> scale_anim_array;
};

struct TextureDataStruct{
	std::string file_name;
	GLuint tex_id {};
};


struct MeshDataStruct{
	int node_index = -1;//index in the node array
	
	std::string name;
	
	//global transforms
	glm::vec3 translation = glm::vec3(0.f);
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 scale = glm::vec3(1.f);
	
	//vertex norm/uv etc data
	std::vector<glm::vec3> vertex_positions_array;
	std::vector<glm::vec3> vertex_normals_array;
	std::vector<glm::vec2> vertex_uvs_array;
	std::vector<unsigned int> vertex_indices_array;
	
	//textures
	GLuint diffuse_texture {};
	GLuint normal_texture {};
	GLuint metal_texture {};
	
	//materials
	bool has_material = false;
	tinygltf::Material material;
	
	//animation data FOR THIS MESH
	AnimationDataStruct animation_data {};
	
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
	glm::mat4 mat(1.f);
	
	//apply translation
	mat = glm::translate(glm::mat4(1.f), position);
	
	//apply rotation
	glm::mat4 rotate_mat = glm::mat4(rotation);
	
	mat = mat * glm::mat4(rotation);
	
	//finally apply scale
	mat = glm::scale(mat, scale);
	
	return mat;
}



#endif //UTILS_H
