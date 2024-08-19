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

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    bool LoadModel(const std::string& path, const std::string& diffuse_tex_name, const std::string& normal_tex_name, const std::string& metallic_tex_name);
    void Render();
		
	long unsigned int getSizeOfComponentType(int component_type);
	
	std::vector<glm::vec3> getVertexPositions() const {	return vertex_positions_array;	}
	std::vector<glm::vec3> getVertexNormals() const {	return vertex_normals_array;	}
	std::vector<glm::vec2> getVertexUVs() const {	return vertex_uvs_array;	}
	std::vector<unsigned int> getIndices() const {	return vertex_indices_array;	}
	
	GLuint getDiffuseTexture() const {	return diffuse_texture;	}
	GLuint getNormalTexture() const {	return normal_texture;	}
	GLuint getMetalTexture() const {	return metal_texture;	}
	
	bool has_diffuse_tex = false;
	bool has_normal_tex = false;
	bool has_metal_tex = false;
	
private:
    tinygltf::TinyGLTF tiny_gltf;
    tinygltf::Model model;
	tinygltf::Mesh mesh;
	
	std::string mesh_name;
	
	unsigned int vertex_count {};
	
	GLuint diffuse_texture;
	GLuint normal_texture;
	GLuint metal_texture;
	
	
	std::vector<glm::vec3> vertex_positions_array;
	std::vector<glm::vec3> vertex_normals_array;
	std::vector<glm::vec2> vertex_uvs_array;
	std::vector<unsigned int> vertex_indices_array;
};

#endif //MODEL_LOADER_H
