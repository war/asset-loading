#ifndef MODEL_LOADER_H //header include guard
#define MODEL_LOADER_H


#include <vector>
#include "../external/tinygltf/tiny_gltf.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>

#include <glad/glad.h>


#include "ErrorLogger.h"

struct AnimationDataStruct{
	std::string name;
	
	int node_index = -1;
	bool has_animation = false;
	
	std::vector<float> time_array;//should be a time for each of trans/rot/scale
	std::vector<glm::vec3> translation_anim_array;
	std::vector<glm::quat> rotation_anim_array;
	std::vector<glm::vec3> scale_anim_array;
};


inline void printGlmVec3(const glm::vec3& v){
	std::cout << "[x: " << v.x << ", y: " << v.y << ", z: " << v.z << "]" << std::endl;
}

inline void printGlmMat4(const glm::mat4& m){
	/*
	*/
	std::cout << "Mat4: " << std::endl;
	std::cout << "[x: " << m[0].x << ", y: " << m[0].y << ", z: " << m[0].z << ", w:" << m[0].w << "]" << std::endl;
	std::cout << "[x: " << m[1].x << ", y: " << m[1].y << ", z: " << m[1].z << ", w:" << m[1].w << "]" << std::endl;
	std::cout << "[x: " << m[2].x << ", y: " << m[2].y << ", z: " << m[2].z << ", w:" << m[2].w << "]" << std::endl;
	std::cout << "[x: " << m[3].x << ", y: " << m[3].y << ", z: " << m[3].z << ", w:" << m[3].w << "]" << std::endl;
	
}

inline void printGlmQuat(const glm::quat& q){
	/*
	*/
	std::cout << "[x: " << q.x << ", y: " << q.y << ", z: " << q.z << ", w:" << q.w << "]" << std::endl;
	
}

inline glm::mat4 createTRSmatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale){
	glm::mat4 mat(1.f);
	
	//apply translation
	mat = glm::translate(glm::mat4(1.f), position);
	
	//apply rotation
	glm::mat4 rotate_mat = glm::mat4(rotation);
	
	mat = glm::translate(glm::mat4(1.f), position) * glm::mat4(rotation);
	
	//finally apply scale
	mat = glm::scale(mat, scale);
	
	return mat;
}


class ModelLoader {
public:
    ModelLoader(const std::string& path, const std::string& diffuse_tex_name, const std::string& normal_tex_name, const std::string& metallic_tex_name);
    ~ModelLoader();

    void Render();
		
		long unsigned int getSizeOfComponentType(int component_type);
		
		//vertex/index/normal/uv data
		std::vector<glm::vec3> getVertexPositions() const {	return vertex_positions_array;	}
		std::vector<glm::vec3> getVertexNormals() const {	return vertex_normals_array;	}
		std::vector<glm::vec2> getVertexUVs() const {	return vertex_uvs_array;	}
		std::vector<unsigned int> getIndices() const {	return vertex_indices_array;	}
		
		//textures
		std::string diffuse_texture_name;
		std::string normal_texture_name;
		std::string metallic_texture_name;
		bool has_diffuse_tex = false;
		bool has_normal_tex = false;
		bool has_metal_tex = false;
		GLuint getDiffuseTexture() const {	return diffuse_texture;	}
		GLuint getNormalTexture() const {	return normal_texture;	}
		GLuint getMetalTexture() const {	return metal_texture;	}
		void generateTextures();
		
		//global model TRS
		glm::vec3 getTranslation() const {	return translation;	}
		glm::quat getRotation() const {	return rotation;	}
		glm::vec3 getScale() const {	return scale;	}
		
		//animations
		bool has_animation = false;
		std::string animation_name;
		tinygltf::Animation animation;
	
		std::vector<AnimationDataStruct> animation_map;//contains list of all animations for this model, with key being the animation name
	
		std::vector<float> time_array;//should be a time for each of trans/rot/scale
		std::vector<glm::vec3> translation_anim_array;
		std::vector<glm::quat> rotation_anim_array;
		std::vector<glm::vec3> scale_anim_array;
	
		//skinning
		bool has_skin = false;
		tinygltf::Skin skin;
		std::vector<glm::vec4> joints_array;
		std::vector<glm::vec4> weights_array;
		std::vector<glm::mat4> inverse_bind_matrix_array;

private:
    tinygltf::TinyGLTF tiny_gltf;
    tinygltf::Model model;
		tinygltf::Mesh mesh;
	
		std::string mesh_name;
		
		unsigned int vertex_count {};
		
		GLuint diffuse_texture;
		GLuint normal_texture;
		GLuint metal_texture;
		
		glm::vec3 translation = glm::vec3(0.f);
		glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
		glm::vec3 scale = glm::vec3(1.f);
		
		std::vector<glm::vec3> vertex_positions_array;
		std::vector<glm::vec3> vertex_normals_array;
		std::vector<glm::vec2> vertex_uvs_array;
		std::vector<unsigned int> vertex_indices_array;
};

#endif //MODEL_LOADER_H
