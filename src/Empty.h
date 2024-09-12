#ifndef EMPTY_H
#define EMPTY_H

#include "../external/tinygltf/tiny_gltf.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>


#include <iostream>
#include <string>
#include <vector>

#include "Utils.h"

class Empty{
public:
	Empty();

	int node_index = -1;
	tinygltf::Node node;
	
	std::string name;
	
	glm::vec3 position = glm::vec3(0.f);
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
	
};



#endif //empty
