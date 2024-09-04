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
	std::vector<MeshDataStruct> getChildMeshArray(const Empty& empty);
	Empty getFirstChildEmpty(const Empty& parent_empty);
	std::vector<Empty> getChildEmptyArray(const Empty& parent_empty);
	
	void updateChildMeshes(const Empty& parent_empty, const glm::mat4& parent_trs);
	
	void equalizeAllAnimationDurations();
	void equalizeAllAnimationDurations2();
	
	void fillAnimationGaps(AnimationDataStruct& animation_data);
	
	void resetAnimations();
	
};







#endif //ANIMATION_PLAYER_H
