#include "AnimationPlayer.h"


AnimationPlayer::AnimationPlayer(ModelLoader* _model, std::vector<Mesh*>* _mesh_array, WindowManager* win_manager) : model(_model), mesh_array(_mesh_array), window_manager(win_manager){
	
	//#############
	//PRINT CHILDS
	//=============
	for(Empty& empty : model->empties_array)
		for(auto c : empty.child_array){
			PRINT_WARN("name :"  + empty.name + ", has child index: " + std::to_string(c) );
		}
	
}

void AnimationPlayer::update(){
	std::vector<Empty>& empty_array = model->empties_array;
	std::vector<MeshDataStruct>& mesh_data_struct_array = model->mesh_data_struct_array;
	
	
//	std::cout << empty_array.front().animation_data.translation_anim_array.size() << std::endl;
	
	for(Empty& empty : empty_array){
		
		AnimationDataStruct& animation_data = empty.animation_data;

		//skip for current empty if no anims found
		if(!empty.animation_data.has_animation)
			continue;
		//add warning message
		if(empty.child_array.size() > 1){
			PRINT_WARN("WARNING -- MORE THAN 1 CHILD");
		}
		
		animation_data.current_animation_time += window_manager->GetDeltaTime() * animation_data.playback_speed;
		
		//UPDATE EMPTY ANIMATED POS/ROT/SCALE
		empty.position = calculateCurrentTranslation(animation_data);
		empty.rotation = calculateCurrentRotation(animation_data);
		empty.scale = calculateCurrentScale(animation_data);
		
		glm::mat4 parent_empty_trs = createTRSmatrix(empty.position, empty.rotation, empty.scale);
		
		
		//GET FIRST EMPTY CHILD [if it exists]
		Empty child_empty = getFirstChildEmpty(empty);
		if(child_empty.node_index != -1){
			
			//////////// NEED TO MULTIPLY MODELMATRIX BY PARENT TRANSFORMS
			glm::vec3 child_curr_anim_position = calculateCurrentTranslation(child_empty.animation_data);
			glm::quat child_curr_anim_rotation = calculateCurrentRotation(child_empty.animation_data);
			glm::vec3 child_curr_anim_scale = calculateCurrentScale(child_empty.animation_data);
			glm::mat4 child_anim_model_matrix = createTRSmatrix(child_curr_anim_position, child_curr_anim_rotation, child_curr_anim_scale);//model matrix of animated pos/rot/scale
			
			glm::mat4 child_model_matrix = createTRSmatrix(child_empty.position, child_empty.rotation, child_empty.scale);//model matrix of static pos/rot/scale
			
			//final model matrix
			child_empty.modelMatrix = child_model_matrix * parent_empty_trs * child_anim_model_matrix;
			
			
			//GET FIRST MESH CHILD [if it exists]
			MeshDataStruct child_msh = getFirstChildMesh(child_empty);
			if(child_msh.node_index != -1){
				
				glm::mat4 mesh_model_martrix = createTRSmatrix(child_msh.translation, child_msh.rotation, child_msh.scale);//model matrix of static pos/rot/scale
				
				//////////////////
				//NEED TO MULTIPLY BY MESH TRANSFORM
				//NEED TO MULTIPLY BY MESH TRANSFORM
				
				//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
				//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
				child_msh.modelMatrix = child_empty.modelMatrix * mesh_model_martrix;
				
				
				for(Mesh* m : *mesh_array)
					if(m->mesh_data.node_index == child_msh.node_index)
						m->mesh_data = child_msh;
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
		
		
		/*
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
		*/
		
		
	}
	
	
//	for(Mesh* mesh : *mesh_array){
//		if(mesh->getMeshData().node_index == FIRST_CHILD_MESH.node_index){
//	Mesh* mesh = mesh_array->front();
//			mesh->setTranslation( empty_array[1].position );
//			mesh->setRotation( empty_array[1].rotation );
//			mesh->setScale( empty_array[1].scale );
//		}
//	}
	
	
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

Empty AnimationPlayer::getFirstChildEmpty(const Empty& empty){
	Empty child_empty;
	
	std::vector<Empty>& empty_array = model->empties_array;
	
	//get first child empty (if any). BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
	for(int c : empty.child_array){
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
