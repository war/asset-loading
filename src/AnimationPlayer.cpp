#include "AnimationPlayer.h"


AnimationPlayer::AnimationPlayer(ModelLoader* _model, std::vector<Mesh*>* _mesh_array, WindowManager* win_manager) : model(_model), mesh_array(_mesh_array), window_manager(win_manager){
	
	//ensures all empty animations have the same duration (required for full animation playback)
	equalizeAllAnimationDurations();
}

void AnimationPlayer::update(){
	std::vector<Empty>& empty_array = model->empties_array;
	std::vector<MeshDataStruct>& mesh_data_struct_array = model->mesh_data_struct_array;
	
//	std::cout << empty_array.front().animation_data.translation_anim_array.size() << std::endl;
	
	
	for(Empty& empty : empty_array){
		
		AnimationDataStruct& animation_data = empty.animation_data;
		
//		std::cout << "name: " <<empty.name << ", childs" << empty.child_array.size() << std::endl;

		//skip for current empty if no anims found
		if(!empty.animation_data.has_animation)
			continue;
		
		/*
		//print childs EMPTY
		for(auto e : getChildEmptyArray(empty)){
			std::cout << "name: " <<empty.name << ", child: " << e.name << std::endl;
		}
		*/

		
		animation_data.current_animation_time += window_manager->GetDeltaTime() * animation_data.playback_speed;
		
		//UPDATE EMPTY ANIMATED POS/ROT/SCALE
		auto empty_position = calculateCurrentTranslation(animation_data);
		auto empty_rotation = calculateCurrentRotation(animation_data);
		auto empty_scale = calculateCurrentScale(animation_data);
		
		glm::mat4 parent_empty_trs = createTRSmatrix(empty_position, empty_rotation, empty_scale);
		
		
		std::cout << getChildEmptyArray(empty).size() << std::endl;
		
		/*
		*/
		//GET FIRST EMPTY CHILD [if it exists]
//		Empty child_empty = getFirstChildEmpty(empty);
		for(Empty& child_empty : getChildEmptyArray(empty)){
			
			if(child_empty.node_index != -1){

				//////////// NEED TO MULTIPLY MODELMATRIX BY PARENT TRANSFORMS
				glm::vec3 child_curr_anim_position = calculateCurrentTranslation(child_empty.animation_data);
				glm::quat child_curr_anim_rotation = calculateCurrentRotation(child_empty.animation_data);
				glm::vec3 child_curr_anim_scale = calculateCurrentScale(child_empty.animation_data);
				glm::mat4 child_anim_model_matrix = createTRSmatrix(child_curr_anim_position, child_curr_anim_rotation, child_curr_anim_scale);//model matrix of animated pos/rot/scale
				
				glm::mat4 child_model_matrix = createTRSmatrix(child_empty.position, child_empty.rotation, child_empty.scale);//model matrix of static pos/rot/scale
				
				//final model matrix
				if(child_empty.animation_data.has_animation)
					child_empty.modelMatrix = parent_empty_trs * child_anim_model_matrix;
				else
					child_empty.modelMatrix = parent_empty_trs * child_model_matrix;
				
				//////////////////
				//////////////////
				//EXTRA CHILD LAYER
				//////////////////
				//////////////////
				for(Empty& child_child_empty : getChildEmptyArray(child_empty)){
					if(child_child_empty.node_index != -1){
						
						//////////// NEED TO MULTIPLY MODELMATRIX BY PARENT TRANSFORMS
						glm::vec3 child_child_curr_anim_position = calculateCurrentTranslation(child_child_empty.animation_data);
						glm::quat child_child_curr_anim_rotation = calculateCurrentRotation(child_child_empty.animation_data);
						glm::vec3 child_child_curr_anim_scale = calculateCurrentScale(child_child_empty.animation_data);
						glm::mat4 child_child_anim_model_matrix = createTRSmatrix(child_child_curr_anim_position, child_child_curr_anim_rotation, child_child_curr_anim_scale);//model matrix of animated pos/rot/scale
						
						glm::mat4 child_child_model_matrix = createTRSmatrix(child_child_empty.position, child_child_empty.rotation, child_child_empty.scale);//model matrix of static pos/rot/scale
						
						//final model matrix
						child_child_empty.modelMatrix = child_empty.modelMatrix * child_child_model_matrix;
						
						//GET FIRST MESH CHILD [if it exists]
						MeshDataStruct child_child_msh = getFirstChildMesh(child_child_empty);
						if(child_child_msh.node_index != -1){
							
							glm::mat4 mesh_model_matrix = createTRSmatrix(child_child_msh.translation, child_child_msh.rotation, child_child_msh.scale);//model matrix of static pos/rot/scale
							//////////////////
							//NEED TO MULTIPLY BY MESH TRANSFORM
							//NEED TO MULTIPLY BY MESH TRANSFORM
							
							//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
							//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
							child_child_msh.modelMatrix = child_child_empty.modelMatrix * mesh_model_matrix;
							
							//mark this as inheriting animation from its root empty objects
							child_child_msh.inherits_animation = true;
							
							for(Mesh* m : *mesh_array)
								if(m->mesh_data.node_index == child_child_msh.node_index){
									m->mesh_data = child_child_msh;
								}
						}
						
						
					}
						
				}
					
				
				//GET FIRST MESH CHILD [if it exists]
				MeshDataStruct child_msh = getFirstChildMesh(child_empty);
				if(child_msh.node_index != -1){
					
					glm::mat4 mesh_model_matrix = createTRSmatrix(child_msh.translation, child_msh.rotation, child_msh.scale);//model matrix of static pos/rot/scale
					//////////////////
					//NEED TO MULTIPLY BY MESH TRANSFORM
					//NEED TO MULTIPLY BY MESH TRANSFORM
					
					//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
					//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
					child_msh.modelMatrix = child_empty.modelMatrix * mesh_model_matrix;
					
					//mark this as inheriting animation from its root empty objects
					child_msh.inherits_animation = true;
					
					for(Mesh* m : *mesh_array)
						if(m->mesh_data.node_index == child_msh.node_index){
							m->mesh_data = child_msh;
						}
				}
				
			}
			
			
		}
		
		
		
		/*
		bool base_reached = false;
		tinygltf::Node node = model->getTinyGltfModel().nodes[ empty.node_index ];
		int tree_level {};
		while(!base_reached){
			if(node.children.empty()){
				break;
			}
			node = model->getTinyGltfModel().nodes[ node.children[0] ];//first child BAD IDEA
			tree_level++;
		}
		std::cout << "levels " <<  tree_level << ", " << empty.name << std::endl;
		*/
		
		
		
	}
	
	
}

glm::vec3 AnimationPlayer::calculateCurrentTranslation(AnimationDataStruct& animation_data){
	
	std::vector<float> time_array = animation_data.time_array;
	
	/////////////////////////////////
	//translation frame lerp
	/////////////////////////////////
	glm::vec3 final_mesh_pos = glm::vec3(0.f);
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
	glm::quat final_mesh_rot = glm::quat(1.f, 0.f, 0.f, 0.f);
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
	
	glm::vec3 final_mesh_scale = glm::vec3(1.f);
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
	std::vector<MeshDataStruct>& mesh_data_struct_array = model->mesh_data_struct_array;
	
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
	
	std::vector<Empty>& empty_array = model->empties_array;
	
	//get first child empty (if any). BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
	for(int c : parent_empty.child_array){
		for(const Empty& e : empty_array){
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
	
	std::vector<Empty>& empty_array = model->empties_array;
	
	//get first child empty (if any). BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
	for(int c : parent_empty.child_array){
		for(const Empty& e : empty_array){
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
	for(Empty& empty : model->empties_array){
		empty.animation_data.current_animation_time = 0.f;
	}
	
	//reset for all Meshes
	for(MeshDataStruct& mesh_data : model->mesh_data_struct_array){
		mesh_data.animation_data.current_animation_time = 0.f;
	}
	
	//reset for skinning
	for(Mesh* mesh : *mesh_array)
		mesh->current_animation_time = 0.f;
	/*
	for(AnimationDataStruct& bone_anim_data : model->bone_animation_array){
		bone_anim_data.current_animation_time = 0.f;//not being used currently
	}
	*/
	
}



void AnimationPlayer::equalizeAllAnimationDurations(){
	
	std::vector<Empty>& empty_array = model->empties_array;
	
	//break out if no empties
	if(empty_array.empty())
		return;
	
	std::map<int, std::vector<float>> size_sorted_timelines;
	
	for(Empty& empty : empty_array){
		AnimationDataStruct& animation_data = empty.animation_data;
		size_sorted_timelines.emplace(animation_data.time_array.size(), animation_data.time_array);
	}
	
	std::vector<float> max_timeline_array = size_sorted_timelines.rbegin()->second;
	
	int max_size = max_timeline_array.size();

	for(Empty& empty : empty_array){
		AnimationDataStruct& animation_data = empty.animation_data;
		animation_data.time_array = max_timeline_array;
		
		equalizeTRSanimationArrays(animation_data);
	}
	
}
