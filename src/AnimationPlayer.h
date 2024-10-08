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
	
	MeshDataStruct* getFirstChildMesh(const EmptyNode* empty);
	EmptyNode* getFirstChildEmptyNode(const EmptyNode* parent_empty);
	std::vector<EmptyNode*> getChildEmptyNodeArray(const EmptyNode* parent_empty);
	
	void setPlaybackSpeed(float _speed);
	void resetAnimations();
	
private:
	ModelLoader* model_loader;
	std::vector<Mesh*>* mesh_array;
	WindowManager* window_manager;
	
//	std::map<std::string, EmptyNode*> empties_map;
	
};







#endif //ANIMATION_PLAYER_H
