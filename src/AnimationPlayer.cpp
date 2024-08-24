#include "AnimationPlayer.h"


AnimationPlayer::AnimationPlayer(ModelLoader* _model, std::vector<Mesh*>* _mesh_array, WindowManager* win_manager) : model(_model), mesh_array(_mesh_array), window_manager(win_manager){
	
}


void AnimationPlayer::update(){
	std::vector<Empty>& empty_array = model->empties_array;
	std::vector<MeshDataStruct>& mesh_data_struct_array = model->mesh_data_struct_array;
	
	
	
//	std::cout << empty_array.front().animation_data.translation_anim_array.size() << std::endl;
	
	for(Empty& empty : empty_array){
		
		AnimationDataStruct& animation_data = empty.animation_data;
		
		animation_data.current_animation_time += window_manager->GetDeltaTime() * animation_data.playback_speed;
		
		
		//UPDATE EMPTY ANIMATED POS/ROT/SCALE
		empty.position = calculateCurrentTranslation(animation_data);
		empty.rotation = calculateCurrentRotation(animation_data);
		empty.scale = calculateCurrentScale(animation_data);
		

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
		
		
		/*
		*/
		//get child mesh BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
		MeshDataStruct FIRST_CHILD_MESH;
		for(auto c : empty.child_array){
			for(auto m :mesh_data_struct_array){
				if(m.node_index == c){
					FIRST_CHILD_MESH = m;
					break;
				}
			}
		}
		
		//SKIP CURR LOOP IF NO CHILD MESH
		if(FIRST_CHILD_MESH.node_index == -1)
			continue;
		
		//update child pos/rot/scale
		for(Mesh* mesh : *mesh_array){
			if(mesh->getMeshData().node_index == FIRST_CHILD_MESH.node_index){
				mesh->setTranslation( empty.position );
				mesh->setRotation( empty.rotation );
				mesh->setScale( empty.scale );
			}
		}
		
		
	}
	
	//set mesh pos
//	mesh_array->front()->setTranslation(glm::vec3(10.f,0.f,0.f));
	
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

