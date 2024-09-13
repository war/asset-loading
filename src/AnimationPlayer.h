#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#include <vector>

#include "./Window/WindowManager.h"
#include "ModelLoader.h"
#include "Utils.h"
#include "Mesh.h"

class AnimationPlayer{
	
public:
	explicit AnimationPlayer(ModelLoader* _model, std::vector<Mesh*>* _mesh_array, WindowManager* win_manager);
	~AnimationPlayer();
	
	void update();
	
	glm::vec3 calculateCurrentTranslation(AnimationDataStruct& animation_data);
	glm::quat calculateCurrentRotation(AnimationDataStruct& animation_data);
	glm::vec3 calculateCurrentScale(AnimationDataStruct& animation_data);
	
	MeshDataStruct getFirstChildMesh(const Empty& empty);
	Empty getFirstChildEmpty(const Empty& parent_empty);
	std::vector<Empty> getChildEmptyArray(const Empty& parent_empty);
	
	void resetAnimations();

private:
	ModelLoader* model_loader;
	std::vector<Mesh*>* mesh_array;
	WindowManager* window_manager;
	
	std::map<std::string, Empty*> empties_map;
	std::vector<std::pair<Empty*, Empty*>> root_and_child_array;
	std::vector<Empty*> root_array;//only will be used for edge cases where root has animations
	
};







#endif //ANIMATION_PLAYER_H
