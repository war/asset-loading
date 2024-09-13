#include "AnimationPlayer.h"

#include <algorithm>

AnimationPlayer::AnimationPlayer(ModelLoader* _model, std::vector<Mesh*>* _mesh_array, WindowManager* win_manager) : model_loader(_model), mesh_array(_mesh_array), window_manager(win_manager){
	
	std::vector<Empty>& empties_array = model_loader->getEmptiesArray();
	std::vector<MeshDataStruct>& mesh_data_struct_array = model_loader->getMeshDataArray();
	
	//gather all root nodes
	std::vector<tinygltf::Node> root_nodes;
	for(const Empty& empty : empties_array)
		if(model_loader->isRootNode(empty.node))
			root_nodes.emplace_back(empty.node);
	
	//fill in empties map
	for(Empty& e : empties_array){
		empties_map.emplace(e.name, &e);
	}
	
	//fill in root-subnode relationship array
	for(Empty& empty : empties_array){
		
		//skip if bone
		if(model_loader->isBone(empty.node_index))
			continue;
		
		int idx = model_loader->getParentNodeIndex(empty.node);
		if(idx == -1)
			continue;
		
		std::pair<Empty*, Empty*> pair;
		
		//find root empty for the child
		for(Empty& e_sub : empties_array){
			if(e_sub.node_index == idx){
				pair.first = &e_sub;
				break;
			}
		}
		
		//child empty
		pair.second = &empty;
		
		//add to array
		root_and_child_array.emplace_back(pair);
	}

	//FOR EDGE CASES -- fill in all root nodes
	for(Empty& empty : empties_array){
		//skip if bone
		if(model_loader->isBone(empty.node_index))
			continue;
		
		int idx = model_loader->getParentNodeIndex(empty.node);
		if(idx == -1)//MUST be == -1 for it to be a root node
			root_array.emplace_back(&empty);
	}
		
	
	
}

AnimationPlayer::~AnimationPlayer(){}

void AnimationPlayer::update(){
	
	std::vector<Empty>& empties_array = model_loader->getEmptiesArray();
	std::vector<MeshDataStruct>& mesh_data_struct_array = model_loader->getMeshDataArray();
	
	const tinygltf::Model& model_tinygltf = model_loader->getTinyGltfModel();
	
	//map key = node index
	std::map<int, Mesh*> mesh_index_map;
	for(Mesh* m  : *mesh_array)
		mesh_index_map.emplace(m->mesh_data.node_index, m);
	
	////////////////////////////////////////
	//EDGE CASE -- update animated ROOT nodes
	////////////////////////////////////////
	for(Empty* root_empty : root_array){
		AnimationDataStruct& animation_data = root_empty->animation_data;
		
		//only interested in animated root nodes
		if(!animation_data.has_animation)
			continue;
		
		animation_data.current_animation_time += window_manager->GetDeltaTime() * animation_data.playback_speed;
		
		//UPDATE EMPTY ANIMATIONS POS/ROT/SCALE [if they exist]
		glm::vec3 anim_position = calculateCurrentTranslation(animation_data);
		glm::quat anim_rotation = calculateCurrentRotation(animation_data);
		glm::vec3 anim_scale = calculateCurrentScale(animation_data);
		
		//animated TRS matrix [if animated]
		glm::mat4 anim_matrix = createTRSmatrix(anim_position, anim_rotation, anim_scale);
		
		root_empty->modelMatrix = anim_matrix;//if this node is animated, its final matrix will be = root matrix * animted TRS matrix
	}
	
	//////////////////////////////////////////////
	//update all model animations [nodes-subnodes]
	//////////////////////////////////////////////
	for(const auto& pair_itr : root_and_child_array){
		Empty* empty_root = pair_itr.first;
		Empty* empty_child = pair_itr.second;
		AnimationDataStruct& anim_data_child = empty_child->animation_data;
		
		glm::mat4 root_matrix = empty_root->modelMatrix;
		
		anim_data_child.current_animation_time += window_manager->GetDeltaTime() * anim_data_child.playback_speed;
		
		//UPDATE EMPTY ANIMATIONS POS/ROT/SCALE [if they exist]
		glm::vec3 child_anim_position = calculateCurrentTranslation(anim_data_child);
		glm::quat child_anim_rotation = calculateCurrentRotation(anim_data_child);
		glm::vec3 child_anim_scale = calculateCurrentScale(anim_data_child);
		
		//animated TRS matrix [if animated]
		glm::mat4 child_anim_matrix = createTRSmatrix(child_anim_position, child_anim_rotation, child_anim_scale);
		
		if(empty_child->animation_data.has_animation){
			empty_child->modelMatrix = root_matrix * child_anim_matrix;//if this node is animated, its final matrix will be = root matrix * animted TRS matrix
		}
		else{
			//if uses matrix instead of raw pos/rot/scale data supplied from gltf
			if(empty_child->has_matrix_transform)
				empty_child->modelMatrix = root_matrix * empty_child->matrix_transform;
			else
				empty_child->modelMatrix = root_matrix * createTRSmatrix(empty_child->translation, empty_child->rotation, empty_child->scale);//if this node is NOT animated, its final matrix will be = root matrix * static TRS matrix
		}
		
		const std::vector<MeshDataStruct>& child_mesh_array = model_loader->getChildMeshArray(model_tinygltf.nodes[empty_child->node_index]);
		
		//update child meshes
		if(!child_mesh_array.empty()){
			for(const MeshDataStruct& msh_data : child_mesh_array){
				if(msh_data.node_index == -1)
					continue;
				
				glm::mat4 mesh_model_matrix = createTRSmatrix(msh_data.translation, msh_data.rotation, msh_data.scale);//model_loader matrix of static pos/rot/scale
				
				mesh_index_map[msh_data.node_index]->mesh_data.modelMatrix = mesh_model_matrix * empty_child->modelMatrix;
				mesh_index_map[msh_data.node_index]->mesh_data.inherits_animation = true;
			}
		}
	}
}

glm::vec3 AnimationPlayer::calculateCurrentTranslation(AnimationDataStruct& animation_data){
	
	std::vector<float> time_array = animation_data.time_array;
	
	/////////////////////////////////
	//translation frame lerp
	/////////////////////////////////
	glm::vec3 final_mesh_pos = glm::vec3(animation_data.translation);
	std::vector<glm::vec3> trans_array = animation_data.translation_anim_array;
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (animation_data.current_animation_time <= new_t) {
			float lerp = 1.f - (new_t - animation_data.current_animation_time) / (new_t - old_t);
			
			glm::vec3 old_pos = glm::vec3(trans_array[i].x, trans_array[i].y, trans_array[i].z);
			glm::vec3 new_pos = glm::vec3(trans_array[i + 1].x, trans_array[i + 1].y, trans_array[i + 1].z);
			
			final_mesh_pos = glm::mix(old_pos, new_pos, lerp);
			
			break;
		}
		
		if ( (i == time_array.size() - 1) || animation_data.current_animation_time >= time_array.back() )
			animation_data.current_animation_time = 0.f;
	}
	return final_mesh_pos;
}

glm::quat AnimationPlayer::calculateCurrentRotation(AnimationDataStruct& animation_data){
	/////////////////////////////////
	//rotation frame lerp
	/////////////////////////////////
	std::vector<float> time_array = animation_data.time_array;
	
	std::vector<glm::quat> rotations_vec = animation_data.rotation_anim_array;
	glm::quat final_mesh_rot = animation_data.rotation;
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (animation_data.current_animation_time <= new_t) {
			float lerp = 1.f - (new_t - animation_data.current_animation_time) / (new_t - old_t);
			
			glm::quat old_rot = glm::quat(rotations_vec[i].w, rotations_vec[i].x, rotations_vec[i].y, rotations_vec[i].z);
			glm::quat new_rot = glm::quat(rotations_vec[i + 1].w, rotations_vec[i + 1].x, rotations_vec[i + 1].y, rotations_vec[i + 1].z);
			
			
			//EDGE CASE
			//TAKE SHORTEST PATH (if dot product yields negative)
			float quat_dot = glm::dot(old_rot, new_rot);
			if( quat_dot < 0.f ){
				new_rot = -new_rot;
			}
			
			final_mesh_rot = glm::mix(old_rot, new_rot, lerp);
			
			//EDGE CASE
			//if dot product is almost 1.f, can lerp between vec4
			if( quat_dot > 0.993f ){
				glm::vec4 old_r = glm::vec4(old_rot.x, old_rot.y, old_rot.z, old_rot.w);
				glm::vec4 new_r = glm::vec4(new_rot.x, new_rot.y, new_rot.z, new_rot.w);
				glm::vec4 lerped_r = glm::mix(old_r, new_r, lerp);
				
				final_mesh_rot = glm::quat(lerped_r.w, lerped_r.x, lerped_r.y, lerped_r.z);//overrites rot 
			}
			
			break;
		}
		
		if ( (i == time_array.size() - 1) || animation_data.current_animation_time >= time_array.back() )
			animation_data.current_animation_time = 0.f;
	}
	return final_mesh_rot;
}

glm::vec3 AnimationPlayer::calculateCurrentScale(AnimationDataStruct& animation_data){
	/////////////////////////////////
	//scale frame lerp
	/////////////////////////////////
	std::vector<float> time_array = animation_data.time_array;
	
	glm::vec3 final_mesh_scale = animation_data.scale;
	std::vector<glm::vec3> scale_array = animation_data.scale_anim_array;
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (animation_data.current_animation_time <= new_t) {
			float lerp = 1.f - (new_t - animation_data.current_animation_time) / (new_t - old_t);
			
			glm::vec3 old_scale = glm::vec3(scale_array[i].x, scale_array[i].y, scale_array[i].z);
			glm::vec3 new_scale = glm::vec3(scale_array[i + 1].x, scale_array[i + 1].y, scale_array[i + 1].z);
			
			final_mesh_scale = glm::mix(old_scale, new_scale, lerp);
			
			break;
		}
		
		if ( (i == time_array.size() - 1) || animation_data.current_animation_time >= time_array.back() )
			animation_data.current_animation_time = 0.f;
	}
	return final_mesh_scale;
}

MeshDataStruct AnimationPlayer::getFirstChildMesh(const Empty& empty){
	MeshDataStruct mesh_data;
	std::vector<MeshDataStruct>& mesh_data_struct_array = model_loader->getMeshDataArray();
	
	//get first child mesh (if any). BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
	for(int c : empty.child_array){
		for(const MeshDataStruct& m : mesh_data_struct_array){
			if(m.node_index == c){
				return m;
			}
		}
	}
	
	//WARNING - RETURNS A USELESS OBJECT
	return mesh_data;
	
}

Empty AnimationPlayer::getFirstChildEmpty(const Empty& parent_empty){
	Empty child_empty;
	
	std::vector<Empty>& empties_array = model_loader->getEmptiesArray();
	
	//get first child empty (if any). BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
	for(int c : parent_empty.child_array){
		for(const Empty& e : empties_array){
			if(e.node_index == c){
				child_empty = e;
				return child_empty;
			}
		}
	}
	
	//WARNING - RETURNS A USELESS OBJECT
	return child_empty;
}

std::vector<Empty> AnimationPlayer::getChildEmptyArray(const Empty& parent_empty){
	std::vector<Empty> child_empty_array;
	
	std::vector<Empty>& empties_array = model_loader->getEmptiesArray();
	
	//get first child empty (if any). BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
	for(int c : parent_empty.child_array){
		for(const Empty& e : empties_array){
			if(e.node_index == c){
				child_empty_array.emplace_back( e );
			}
		}
	}
	
	//WARNING - RETURNS A USELESS OBJECT
	return child_empty_array;
}

void AnimationPlayer::resetAnimations(){
	//reset for all Empties
	for(Empty& empty : model_loader->getEmptiesArray()){
		empty.animation_data.current_animation_time = 0.f;
	}
	
	//reset for all Meshes
	for(MeshDataStruct& mesh_data : model_loader->getMeshDataArray()){
		mesh_data.animation_data.current_animation_time = 0.f;
	}
	
	//reset for skinning
	for(Mesh* mesh : *mesh_array)
		mesh->current_animation_time = 0.f;
	/*
	for(AnimationDataStruct& bone_anim_data : model_loader->bone_animation_array){
		bone_anim_data.current_animation_time = 0.f;//not being used currently
	}
	*/
	
}
