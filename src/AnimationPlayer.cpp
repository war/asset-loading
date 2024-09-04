#include "AnimationPlayer.h"

#include <algorithm>


AnimationPlayer::AnimationPlayer(ModelLoader* _model, std::vector<Mesh*>* _mesh_array, WindowManager* win_manager) : model(_model), mesh_array(_mesh_array), window_manager(win_manager){
	
	//ensures all empty animations have the same duration (required for full animation playback)
	
	std::vector<Empty>& empty_array = model->empties_array;
	std::vector<MeshDataStruct>& mesh_data_struct_array = model->mesh_data_struct_array;
	for(Empty& empty : empty_array){
		
		AnimationDataStruct& animation_data = empty.animation_data;
		
		if(!animation_data.has_animation)
			continue;
		
		//fills in discontinuity
		model->fillAnimationGaps(animation_data);
		
		//sloppy call. Equalizes pos/rot/scale arrays to be of equal length
		model->equalizeTRSanimationArrays(animation_data);
		
	}
	
	model->getHierarchy(empty_array[2].node);
	
	
	//Equalizes all animation channels to match the MAX timeline duration
//	equalizeAllAnimationDurations();
	equalizeAllAnimationDurations2();
	
}


void AnimationPlayer::update(){
	std::vector<Empty>& empty_array = model->empties_array;
	std::vector<MeshDataStruct>& mesh_data_struct_array = model->mesh_data_struct_array;
	
//	std::cout << empty_array.front().animation_data.translation_anim_array.size() << std::endl;
	
	std::map<std::string, Empty> empty_map;
	for(Empty& e  : empty_array){
		empty_map.emplace(e.name, e);
	}
	std::map<std::string, Mesh*> mesh_map;
	std::map<int, Mesh*> mesh_index_map;
	for(Mesh* m  : *mesh_array){
		mesh_map.emplace(m->mesh_data.name, m);
		mesh_index_map.emplace(m->mesh_data.node_index, m);
	}
	
	
	
	/*
	auto c769c5abf1f401eac90b76f4b765ba0 = empty_array[0];
	auto Sketchfab_model = empty_array[6];
	
	model->getHierarchy(c769c5abf1f401eac90b76f4b765ba0.node);
	
	//BPPISTOL EMPTY
//	Empty& bppistol = empty_map["BPpistol"];
	Empty& bppistol = empty_array[8];
//	Empty& bppistol = empty_array[10];
	AnimationDataStruct& bppistol_animation = bppistol.animation_data;
	bppistol_animation.current_animation_time += window_manager->GetDeltaTime() * bppistol_animation.playback_speed;
	//UPDATE EMPTY ANIMATED POS/ROT/SCALE
	auto bppistol_position = calculateCurrentTranslation(bppistol_animation);
	auto bppistol_rotation = calculateCurrentRotation(bppistol_animation);
	auto bppistol_scale = calculateCurrentScale(bppistol_animation);
	glm::mat4 bppistol_trs = createTRSmatrix(bppistol_position, bppistol_rotation, bppistol_scale) * c769c5abf1f401eac90b76f4b765ba0.base_matrix;
	
	
	//BPPISTOL BASE MESH
	Mesh* bppistol_mesh = mesh_map["m16a2_base_m16a2_0"];
	MeshDataStruct& child_msh = bppistol_mesh->mesh_data;
	glm::mat4 mesh_model_matrix = createTRSmatrix(child_msh.translation, child_msh.rotation, child_msh.scale);//model matrix of static pos/rot/scale
	child_msh.modelMatrix = bppistol_trs * mesh_model_matrix;
	bppistol_mesh->mesh_data.inherits_animation = true;
	
	//BPPISTOL EMPTY
//	Empty& bppistol = empty_map["BPpistol"];
	Empty& bppistol = empty_array[6];
//	Empty& bppistol = empty_array[10];
	AnimationDataStruct& bppistol_animation = bppistol.animation_data;
	bppistol_animation.current_animation_time += window_manager->GetDeltaTime() * bppistol_animation.playback_speed;
	//UPDATE EMPTY ANIMATED POS/ROT/SCALE
	auto bppistol_position = calculateCurrentTranslation(bppistol_animation);
	auto bppistol_rotation = calculateCurrentRotation(bppistol_animation);
	auto bppistol_scale = calculateCurrentScale(bppistol_animation);
	glm::mat4 bppistol_trs = createTRSmatrix(bppistol_position, bppistol_rotation, bppistol_scale);

	//BASE EMPTY
//	Empty& base = empty_map["base"];
	Empty& base = empty_array[7];
//	Empty& base = empty_array[1];
	AnimationDataStruct& base_animation = base.animation_data;
	base_animation.current_animation_time += window_manager->GetDeltaTime() * base_animation.playback_speed;
	//UPDATE EMPTY ANIMATED POS/ROT/SCALE
	auto base_position = calculateCurrentTranslation(base_animation);
	auto base_rotation = calculateCurrentRotation(base_animation);
	auto base_scale = calculateCurrentScale(base_animation);
	glm::mat4 base_anim_trs = createTRSmatrix(base_position, base_rotation, base_scale);
	
	glm::mat4 base_trs = createTRSmatrix(base.position, base.rotation, base.scale);
	
	glm::mat4 base_FINAL_trs = glm::mat4(1.f);
	(base.animation_data.has_animation) ? base_FINAL_trs = bppistol_trs * base_anim_trs : base_FINAL_trs = bppistol_trs * base_trs;
	
	//BPPISTOL BASE MESH
	Mesh* bppistol_mesh = mesh_map["base_Mat_0"];
	MeshDataStruct& child_msh = bppistol_mesh->mesh_data;
	glm::mat4 mesh_model_matrix = createTRSmatrix(child_msh.translation, child_msh.rotation, child_msh.scale);//model matrix of static pos/rot/scale
	child_msh.modelMatrix = base_FINAL_trs * mesh_model_matrix;
	bppistol_mesh->mesh_data.inherits_animation = true;
	
	/////////////////////////////
	/////////////////////////////
	//CLIP EMPTY
	Empty& clip = empty_array[8];
//	Empty& clip = empty_array[4];
	AnimationDataStruct& clip_animation = clip.animation_data;
	clip_animation.current_animation_time += window_manager->GetDeltaTime() * clip_animation.playback_speed;
	//UPDATE EMPTY ANIMATED POS/ROT/SCALE
	auto clip_position = calculateCurrentTranslation(clip_animation);
	auto clip_rotation = calculateCurrentRotation(clip_animation);
	auto clip_scale = calculateCurrentScale(clip_animation);
	glm::mat4 clip_anim_trs = createTRSmatrix(clip_position, clip_rotation, clip_scale);
	
	glm::mat4 clip_trs = createTRSmatrix(clip.position, clip.rotation, clip.scale);
	
	glm::mat4 clip_FINAL_trs = glm::mat4(1.f);
	(clip.animation_data.has_animation) ? clip_FINAL_trs = bppistol_trs * clip_anim_trs : clip_FINAL_trs = bppistol_trs * clip_trs;
	
	//CLIP [MAG] MESH
	Mesh* clip_mesh = mesh_map["clip_Mat_0"];
	MeshDataStruct& clip_msh = clip_mesh->mesh_data;
	glm::mat4 clip_model_matrix = createTRSmatrix(clip_msh.translation, clip_msh.rotation, clip_msh.scale);//model matrix of static pos/rot/scale
	clip_msh.modelMatrix = clip_FINAL_trs * clip_model_matrix;
	clip_mesh->mesh_data.inherits_animation = true;
	
	/////////////////////
	/////////////////////
	//BULLET EMPTY
	{
		Empty& bullet = empty_array[10];
	//	Empty& bullet = empty_array[2];
		AnimationDataStruct& bullet_animation = bullet.animation_data;
		bullet_animation.current_animation_time += window_manager->GetDeltaTime() * bullet_animation.playback_speed;
		//UPDATE EMPTY ANIMATED POS/ROT/SCALE
		auto bullet_position = calculateCurrentTranslation(bullet_animation);
		auto bullet_rotation = calculateCurrentRotation(bullet_animation);
		auto bullet_scale = calculateCurrentScale(bullet_animation);
		glm::mat4 bullet_anim_trs = createTRSmatrix(bullet_position, bullet_rotation, bullet_scale);
		glm::mat4 bullet_trs = createTRSmatrix(bullet.position, bullet.rotation, bullet.scale);
		glm::mat4 bullet_FINAL_trs = glm::mat4(1.f);
		(bullet.animation_data.has_animation) ? bullet_FINAL_trs = clip_FINAL_trs * bullet_anim_trs : bullet_FINAL_trs = clip_FINAL_trs * bullet_trs;
		
		//BULLET MESH
		Mesh* bullet_mesh = mesh_map["bullet_Mat_0"];
		MeshDataStruct& bullet_msh = bullet_mesh->mesh_data;
		glm::mat4 bullet_model_matrix = createTRSmatrix(bullet_msh.translation, bullet_msh.rotation, bullet_msh.scale);//model matrix of static pos/rot/scale
		bullet_msh.modelMatrix = bullet_FINAL_trs * bullet_model_matrix;
		bullet_mesh->mesh_data.inherits_animation = true;
	}
	/////////////////////
	/////////////////////
	//BULLET_1
	{
		//BULLET_1 MESH
		Mesh* bullet_mesh = mesh_map["bullet_1_Mat_0"];
		MeshDataStruct& bullet_msh = bullet_mesh->mesh_data;
		glm::mat4 bullet_model_matrix = createTRSmatrix(bullet_msh.translation, bullet_msh.rotation, bullet_msh.scale);//model matrix of static pos/rot/scale
		bullet_msh.modelMatrix = clip_FINAL_trs * bullet_model_matrix;
		bullet_mesh->mesh_data.inherits_animation = true;
	}
	
	for(int i{};i<empty_array.size();i++){
		std::cout << empty_array[i].name << ", " << i << std::endl;
	}
	*/
	
	
	
	
	for(Empty& empty : empty_array){
		
		AnimationDataStruct& animation_data = empty.animation_data;
		
		//skip for current empty if no anims or subnodes found
		if(!empty.animation_data.has_animation || empty.child_array.empty())
			continue;
		
		animation_data.current_animation_time += window_manager->GetDeltaTime() * animation_data.playback_speed;
		
		//UPDATE EMPTY ANIMATED POS/ROT/SCALE
		glm::vec3 empty_position = calculateCurrentTranslation(animation_data);
		glm::quat empty_rotation = calculateCurrentRotation(animation_data);
		glm::vec3 empty_scale = calculateCurrentScale(animation_data);
		
		glm::mat4 parent_empty_trs = createTRSmatrix(empty_position, empty_rotation, empty_scale);
		
		
		///////////////
		//REMOVE
//		if(empty.name != "m16a2_base")
		if(empty.name != "BPpistol")
			continue;
		
		///////////////
		//NODE LAYER 1
		for(Empty& child_1_empty : getChildEmptyArray(empty)){
			AnimationDataStruct& child_animation_data = child_1_empty.animation_data;
			
			child_1_empty.inherits_animation = true;
			
			child_animation_data.current_animation_time += window_manager->GetDeltaTime() * child_animation_data.playback_speed;
			
			//UPDATE EMPTY ANIMATED POS/ROT/SCALE
			glm::vec3 child_1_empty_position = calculateCurrentTranslation(child_animation_data);
			glm::quat child_1_empty_rotation = calculateCurrentRotation(child_animation_data);
			glm::vec3 child_1_empty_scale = calculateCurrentScale(child_animation_data);
			
			glm::mat4 child_1_anim_model_matrix = createTRSmatrix(child_1_empty_position, child_1_empty_rotation, child_1_empty_scale);
			
			glm::mat4 child_1_model_matrix = createTRSmatrix(child_1_empty.position, child_1_empty.rotation, child_1_empty.scale);//model matrix of static pos/rot/scale
			
			//final model matrix
			if(child_1_empty.animation_data.has_animation)
				child_1_empty.modelMatrix = parent_empty_trs * child_1_anim_model_matrix;
			else
				child_1_empty.modelMatrix = parent_empty_trs * child_1_model_matrix;
			
			
			
			///////////////
			//NODE LAYER 2
			for(Empty& child_2_empty : getChildEmptyArray(child_1_empty)){
				AnimationDataStruct& child_2_animation_data = child_2_empty.animation_data;
				
				child_2_empty.inherits_animation = true;
				
				child_2_animation_data.current_animation_time += window_manager->GetDeltaTime() * child_2_animation_data.playback_speed;
				
				//UPDATE EMPTY ANIMATED POS/ROT/SCALE
				glm::vec3 child_2_empty_position = calculateCurrentTranslation(child_2_animation_data);
				glm::quat child_2_empty_rotation = calculateCurrentRotation(child_2_animation_data);
				glm::vec3 child_2_empty_scale = calculateCurrentScale(child_2_animation_data);
				
				glm::mat4 child_2_anim_model_matrix = createTRSmatrix(child_2_empty_position, child_2_empty_rotation, child_2_empty_scale);
				
				glm::mat4 child_2_model_matrix = createTRSmatrix(child_2_empty.position, child_2_empty.rotation, child_2_empty.scale);//model matrix of static pos/rot/scale
				
				//final model matrix
				if(child_2_empty.animation_data.has_animation)
					child_2_empty.modelMatrix = child_1_empty.modelMatrix * child_2_anim_model_matrix;
				else
					child_2_empty.modelMatrix = child_1_empty.modelMatrix * child_2_model_matrix;
				
				//update any child meshes
				updateChildMeshes(child_2_empty, parent_empty_trs);
			}
			
			
			//update any child meshes
			updateChildMeshes(child_1_empty, parent_empty_trs);
		}
		
		
		//update any child meshes
		updateChildMeshes(empty, parent_empty_trs);
//		auto g = getFirstChildMesh(empty);
//		PRINT_WARN(g.name);
//		std::cout << empty.name << ", cnt" << empty.child_array.size() << std::endl;
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
	
void AnimationPlayer::updateChildMeshes(const Empty& parent_empty, const glm::mat4& parent_trs){
	MeshDataStruct child_msh = getFirstChildMesh(parent_empty);
	if(child_msh.node_index == -1)
		return;
	
	std::map<int, Mesh*> mesh_index_map;
	for(Mesh* m  : *mesh_array){
		mesh_index_map.emplace(m->mesh_data.node_index, m);
	}
	
	glm::mat4 mesh_model_matrix = createTRSmatrix(child_msh.translation, child_msh.rotation, child_msh.scale);//model matrix of static pos/rot/scale
	//////////////////
	//NEED TO MULTIPLY BY MESH TRANSFORM
	//NEED TO MULTIPLY BY MESH TRANSFORM
	
	//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
	//ADD MESH ANIMS X MULTPLY BY TRANSFORMS
	child_msh.modelMatrix = parent_empty.modelMatrix * mesh_model_matrix;
	
	//mark this as inheriting animation from its root empty objects
	child_msh.inherits_animation = true;
	
	mesh_index_map[child_msh.node_index]->mesh_data = child_msh;
}
	
std::vector<MeshDataStruct> AnimationPlayer::getChildMeshArray(const Empty& empty){
	std::vector<MeshDataStruct> child_mesh_array;
	
//	std::vector<MeshDataStruct>& empty_array = model->empties_array;
//	
//	//get first child empty (if any). BAD IDEA, NEED TO TAKE INTO ACC MULTIPLE CHILDS
//	for(int c : parent_empty.child_array){
//		for(const Empty& e : empty_array){
//			if(e.node_index == c){
//				child_empty_array.emplace_back( e );
//			}
//		}
//	}
//	
	//WARNING - RETURNS A USELESS OBJECT
	return child_mesh_array;
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
	
	for(Empty& empty : empty_array){
		AnimationDataStruct& animation_data = empty.animation_data;
		animation_data.time_array = max_timeline_array;
		
		equalizeTRSanimationArrays(animation_data);
	}
	
}


void AnimationPlayer::equalizeAllAnimationDurations2(){
	
	std::vector<Empty>& empty_array = model->empties_array;
	
	//break out if no empties
	if(empty_array.empty())
		return;
	
	std::vector<int> size_sorted_timeline_sizes;
	std::map<int, std::vector<float>> timeline_map;//only used to calc delta time
	
	for(Empty& empty : empty_array){
		AnimationDataStruct& animation_data = empty.animation_data;
		size_sorted_timeline_sizes.emplace_back(animation_data.translation_anim_array.size());
		size_sorted_timeline_sizes.emplace_back(animation_data.rotation_anim_array.size());
		size_sorted_timeline_sizes.emplace_back(animation_data.scale_anim_array.size());
		
		timeline_map.emplace(animation_data.translation_anim_array.size(), animation_data.trans_time_array);
		timeline_map.emplace(animation_data.rotation_anim_array.size(), animation_data.rot_time_array);
		timeline_map.emplace(animation_data.scale_anim_array.size(), animation_data.scale_time_array);
	}
	
	if(timeline_map.empty() || size_sorted_timeline_sizes.empty())
		return;
	
	std::sort(size_sorted_timeline_sizes.begin(), size_sorted_timeline_sizes.end());
	
	int max_timeline_size = size_sorted_timeline_sizes.back();
	
	std::vector<float> max_timeline_array;
	
	float time_cnt {};
	
	//get delta time [bad method]
	//get delta time [bad method]
	if(timeline_map.rbegin()->second.empty())
		throw std::logic_error("bad delta_time, empty array FIX");
	
//	float delta_time = timeline_map.rbegin()->second.back()/max_timeline_size;
	float delta_time = DELTA_TIME_CHANGE_THIS;
	
		
	
	for(int i{}; i<max_timeline_size; i++){
		
		max_timeline_array.emplace_back(time_cnt);
		
		time_cnt += delta_time;
	}
	
	for(Empty& empty : empty_array){
		AnimationDataStruct& animation_data = empty.animation_data;
		animation_data.time_array = max_timeline_array;
		
		equalizeTRSanimationArrays(animation_data);
	}
	
}
