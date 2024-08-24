#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#include <vector>

#include "./Window/WindowManager.h"
#include "ModelLoader.h"
#include "Utils.h"
#include "Empty.h"
#include "Mesh.h"

class AnimationPlayer{
public:
	
	ModelLoader* model;
	std::vector<Mesh*>* mesh_array;
	WindowManager* window_manager;
	
	AnimationPlayer(ModelLoader* _model, std::vector<Mesh*>* _mesh_array, WindowManager* win_manager);
	
	void update();
	
	glm::vec3 calculateCurrentTranslation(AnimationDataStruct& animation_data);
	glm::quat calculateCurrentRotation(AnimationDataStruct& animation_data);
	glm::vec3 calculateCurrentScale(AnimationDataStruct& animation_data);
	
	MeshDataStruct getFirstChildMesh(const Empty& empty);
	Empty getFirstChildEmpty(const Empty& empty);
	
};







#endif //ANIMATION_PLAYER_H
