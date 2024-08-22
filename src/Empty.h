#ifndef EMPTY_H
#define EMPTY_H

#include "../external/tinygltf/tiny_gltf.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>


#include <iostream>
#include <vector>

#include "Utils.h"

class Empty{
public:
	Empty();
	
	
	bool has_animation = false;
	std::string animation_name;
	
	tinygltf::Animation anim_gltf;
	
	AnimationDataStruct animation_data;
	
	bool has_root = false;
	int root_idx = -1;
	
};



#endif //empty
