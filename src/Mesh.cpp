#include "Mesh.h"


Mesh::Mesh(Camera* cam, ModelLoader* model_loader, MeshDataStruct _mesh_data, Shader* _shader, WindowManager* win_manager, DirectionalLight* _direct_light) : camera(cam), model(model_loader), mesh_data(_mesh_data), shader(_shader), window_manager(win_manager), direct_light(_direct_light){
	
	std::vector<glm::vec3> tangent_array;
	std::vector<glm::vec3> bitangent_array;
	
	std::map<unsigned int, glm::vec3> tangent_map;
	std::map<unsigned int, glm::vec3> bitangent_map;
	
	
	std::vector<glm::vec3> vert_pos_vec = mesh_data.vertex_positions_array;
	std::vector<glm::vec2> uv_coord_vec = mesh_data.vertex_uvs_array;
	
	for(int i{}; i<mesh_data.vertex_indices_array.size()/3; i++){
		
		for(int count{}; count<3; count++){
			int vert0_idx = mesh_data.vertex_indices_array[(i*3)+0];
			
			int vert1_idx = mesh_data.vertex_indices_array[(i*3)+1];
			
			int	vert2_idx = mesh_data.vertex_indices_array[(i*3)+2];
			
			glm::vec3 edge0 = glm::vec3( vert_pos_vec[vert1_idx].x, vert_pos_vec[vert1_idx].y, vert_pos_vec[vert1_idx].z ) - glm::vec3( vert_pos_vec[vert0_idx].x, vert_pos_vec[vert0_idx].y, vert_pos_vec[vert0_idx].z );
			glm::vec3 edge1 = glm::vec3( vert_pos_vec[vert2_idx].x, vert_pos_vec[vert2_idx].y, vert_pos_vec[vert2_idx].z ) - glm::vec3( vert_pos_vec[vert0_idx].x, vert_pos_vec[vert0_idx].y, vert_pos_vec[vert0_idx].z );
			
			glm::vec2 uv0 = glm::vec2( uv_coord_vec[vert1_idx].x, uv_coord_vec[vert1_idx].y ) - glm::vec2( uv_coord_vec[vert0_idx].x, uv_coord_vec[vert0_idx].y );
			glm::vec2 uv1 = glm::vec2( uv_coord_vec[vert2_idx].x, uv_coord_vec[vert2_idx].y ) - glm::vec2( uv_coord_vec[vert0_idx].x, uv_coord_vec[vert0_idx].y );
			
			float div = 1.f/(uv0.x * uv1.y - uv1.x * uv0.y);
			
			glm::vec3 tangent(0.f);
			glm::vec3 bitangent(0.f);
			
			tangent.x = div * (uv1.y*edge0.x - uv0.y*edge1.x);
			tangent.y = div * (uv1.y*edge0.y - uv0.y*edge1.y);
			tangent.z = div * (uv1.y*edge0.z - uv0.y*edge1.z);
			
			bitangent.x = div * (-uv1.x*edge0.x + uv0.x*edge1.x);
			bitangent.y = div * (-uv1.x*edge0.y + uv0.x*edge1.y);
			bitangent.z = div * (-uv1.x*edge0.z + uv0.x*edge1.z);
//			
			tangent_map.emplace(mesh_data.vertex_indices_array[(i*3) + count], tangent);
			bitangent_map.emplace(mesh_data.vertex_indices_array[(i*3) + count], bitangent);
			
		}
		
	}
	
	for(const auto& tang : tangent_map){
		tangent_array.emplace_back(tang.second);
	}
	for(const auto& bitang : bitangent_map){
		bitangent_array.emplace_back(bitang.second);
	}
	
	
	
	
	
	
	
	
	//fill in the arrays with flattened GLfloat and GLuint vertex/index data
	for(unsigned int i{}; i<mesh_data.vertex_positions_array.size(); i++){
		
		glm::vec3& vert_pos = mesh_data.vertex_positions_array[i];
		glm::vec2& vert_uv = mesh_data.vertex_uvs_array[i];
		glm::vec3& vert_norm = mesh_data.vertex_normals_array[i];
		
		//push vert positions
		tri_vertices.emplace_back( (GLfloat)vert_pos.x );
		tri_vertices.emplace_back( (GLfloat)vert_pos.y );
		tri_vertices.emplace_back( (GLfloat)vert_pos.z );
		
		//push vert uvs
		tri_vertices.emplace_back( (GLfloat)vert_uv.x );
		tri_vertices.emplace_back( (GLfloat)vert_uv.y );
		
		//push vert normals
		tri_vertices.emplace_back( (GLfloat)vert_norm.x );
		tri_vertices.emplace_back( (GLfloat)vert_norm.y );
		tri_vertices.emplace_back( (GLfloat)vert_norm.z );
		
		//tang
		glm::vec3 tangent = tangent_array[i];
		tri_vertices.emplace_back( 0.f );
		tri_vertices.emplace_back( 1.f );
		tri_vertices.emplace_back( 0.f );
		
		//bitang
		glm::vec3 bitangent = bitangent_array[i];
		tri_vertices.emplace_back( 1.f );
		tri_vertices.emplace_back( 0.f );
		tri_vertices.emplace_back( 0.f );
		
		if(mesh_data.has_skin){
			//joints
			{
				glm::vec4 joint = mesh_data.joints_array[i];
				
				tri_vertices.emplace_back( joint.x );
				tri_vertices.emplace_back( joint.y );
				tri_vertices.emplace_back( joint.z );
				tri_vertices.emplace_back( joint.w );
			}
			//weights
			{
				glm::vec4 weight = mesh_data.weights_array[i];
				
				tri_vertices.emplace_back( weight.x );
				tri_vertices.emplace_back( weight.y );
				tri_vertices.emplace_back( weight.z );
				tri_vertices.emplace_back( weight.w );
			}
		}
		

		
		
	}
	//indices
	for(unsigned int v_idx : mesh_data.vertex_indices_array){
		tri_indices.emplace_back( (GLuint)v_idx );
	}

	vao.bind();
	vbo = VBO(tri_vertices);
	ebo = EBO(tri_indices);
	
	//setup attributes
	if(mesh_data.has_skin){
		//setup pointers to the vertex position data `layout (location = 0)`
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), static_cast<void*>(0));
		glEnableVertexAttribArray(0);
		
		//setup pointers to the vertex UV coord data `layout (location = 1)`
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 3*sizeof(float) ));
		glEnableVertexAttribArray(1);
		
		//setup pointers to the vertex normal data `layout (location = 2)`
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 5*sizeof(float) ));
		glEnableVertexAttribArray(2);
		
		//setup pointers to the vertex tangent data `layout (location = 3)`
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 8*sizeof(float) ));
		glEnableVertexAttribArray(3);
		
		//setup pointers to the vertex bitangent data `layout (location = 4)`
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 11*sizeof(float) ));
		glEnableVertexAttribArray(4);
		
		//setup pointers to the JOINTS_0 data `layout (location = 5)` 
		//USE CHAR INSTEAD OF FLOAT
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 14*sizeof(float) ));
		glEnableVertexAttribArray(5);
		
		//setup pointers to the WEIGHTS_0 data `layout (location = 6)` 
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 18*sizeof(float) ));
		glEnableVertexAttribArray(6);	
		
	}
	//non-skinned mesh
	else{
		//setup pointers to the vertex position data `layout (location = 0)`
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), static_cast<void*>(0));
		glEnableVertexAttribArray(0);
		
		//setup pointers to the vertex UV coord data `layout (location = 1)`
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 14*sizeof(float), reinterpret_cast<void*>( 3*sizeof(float) ));
		glEnableVertexAttribArray(1);
		
		//setup pointers to the vertex normal data `layout (location = 2)`
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), reinterpret_cast<void*>( 5*sizeof(float) ));
		glEnableVertexAttribArray(2);
		
		//setup pointers to the vertex tangent data `layout (location = 3)`
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), reinterpret_cast<void*>( 8*sizeof(float) ));
		glEnableVertexAttribArray(3);
		
		//setup pointers to the vertex bitangent data `layout (location = 4)`
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), reinterpret_cast<void*>( 11*sizeof(float) ));
		glEnableVertexAttribArray(4);
	}
	
	
	vbo.unbind();
	vao.unbind();
	ebo.unbind();
	
	//only throws error if it failed to fill in buffers
	__GL_ERROR_THROW__("Failed to generate Mesh object"); //check for any GL errors
	
	//set position/scale/orn
	position = mesh_data.translation;
	rotation = mesh_data.rotation;
	scale = mesh_data.scale;
	
}

Mesh::~Mesh(){
	//delete VAO, VBO, EBO to avoid mem leaks
	vao.free();
	vbo.free();
	ebo.free();
	
	PRINT_COLOR( "Mesh object [" + mesh_data.name + "] destroyed", 40, 220, 90);
}

void Mesh::update(){
	
	//apply translation
	modelMatrix = glm::translate(glm::mat4(1.f), position);

	//apply rotation
	glm::mat4 rotate_mat = glm::mat4(rotation);
	
	modelMatrix = glm::translate(glm::mat4(1.f), position) * glm::mat4(rotation);

	//finally apply scale
	modelMatrix = glm::scale(modelMatrix, scale);
	
	//if this mesh is a child of an empty, the use the calculated modelMatrix [calculated inside AnimationPlayer.cpp, based off of root empty animations]
	if(mesh_data.inherits_animation)
		modelMatrix = mesh_data.modelMatrix;
	
	//update animations
	updateAnimation();
	
	//bind VAO
	glBindVertexArray(vao.vao);
	
	//send camera and model matrices
	shader->use();
	shader->setMat4("modelMatrix", modelMatrix);
	shader->setMat4("viewMatrix", camera->GetViewMatrix());
	shader->setMat4("projMatrix", glm::perspective(camera->FovRads, window_manager->getAspectRatio(), 0.1f, 1000.0f));
	shader->setVec3("cameraPos", camera->GetPosition());
	
	//light info
	shader->setFloat("light_strength", direct_light->strength);
	shader->setFloat("light_specular", direct_light->specular);
	shader->setVec3("light_color", direct_light->color);
	shader->setVec3("light_dir", direct_light->direction);
	
	///////////
	//textures
	///////////
	//diffuse tex
	if(mesh_data.texture_map.count(TextureType::DIFFUSE) != 0){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh_data.texture_map[TextureType::DIFFUSE].tex_id);
		shader->setInt("diffuse_tex", 0);//send Image to frag shader
	}
	
	//normal tex
	if(mesh_data.texture_map.count(TextureType::NORMAL) != 0){
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mesh_data.texture_map[TextureType::NORMAL].tex_id);
		shader->setInt("normal_tex", 1);//send Image to frag shader
	}
	
	//metal tex
	if(mesh_data.texture_map.count(TextureType::METAL) != 0){
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mesh_data.texture_map[TextureType::METAL].tex_id);
		shader->setInt("metal_tex", 2);//send Image to frag shader
	}
	
	//send base/diffuse color
	shader->setVec3("base_color", glm::vec3( mesh_data.material_data.base_color.x, mesh_data.material_data.base_color.y, mesh_data.material_data.base_color.z ) );
	
	
	//////////
	//skinning
	//////////
	//send skinned matrices to shader
	for(short m{}; m<bone_skinned_matrix_array.size(); m++){
		std::string uniform_name = std::string("inverseBindMatrixArray[") + std::to_string(m) + std::string("].matrix");
		shader->setMat4(uniform_name.c_str(), bone_skinned_matrix_array[m]);
	}
	
	//send isSkinned var
	shader->setInt("isSkinned", (int)mesh_data.has_skin);
	
	//backface culling (if enabled)
	(enable_backface_culling) ? glCullFace(GL_BACK) : glCullFace(GL_BACK);
	(enable_backface_culling) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	
	//render mesh
	glDrawElements(GL_TRIANGLES, tri_indices.size(), GL_UNSIGNED_INT, 0);//rendering part
	
	//checks for rendering errors
	__GL_ERROR_THROW__("Failed to render Mesh.");
}

void Mesh::updateAnimation(){
	
	//old step interpolation method

	/*
	current_animation_frame += 1;
	
	//reset frame counter if end of animation reached
	if(current_animation_frame == model->time_array.size() - 1)
		current_animation_frame = 0;
	
	position = model->translation_anim_array[current_animation_frame];
	rotation = model->rotation_anim_array[current_animation_frame];
	scale = model->scale_anim_array[current_animation_frame];
	
	*/
	
	//update animation time
	current_animation_time += window_manager->GetDeltaTime() * playback_speed;
	
	updateSkinnedAnimation();
	
	AnimationDataStruct& animation_data = mesh_data.animation_data;
	
	//quit if no animations for this mesh
	if(!animation_data.has_animation){
//		PRINT_WARN("no anim") ;
		return;
	}
	
	//ensure TRS anims are of equal lengths
	if( animation_data.translation_anim_array.size() != animation_data.rotation_anim_array.size() || animation_data.translation_anim_array.size() != animation_data.scale_anim_array.size() || animation_data.rotation_anim_array.size() != animation_data.scale_anim_array.size() )
		PRINT_WARN("Translation, scale and rotation animation durations must be equal.");
	
	//apply lerping between frame to set current pos/rot/scale
	glm::vec3 anim_position = calculateCurrentTranslation(animation_data);
	glm::quat anim_rotation = calculateCurrentRotation(animation_data);
	glm::vec3 anim_scale = calculateCurrentScale(animation_data);
	
	modelMatrix = createTRSmatrix(anim_position, anim_rotation, anim_scale);
	
//	std::cout << animation_data.translation_anim_array.size() << std::endl;
		
	
	
}

glm::vec3 Mesh::calculateCurrentTranslation(const AnimationDataStruct& animation_data){
	
	std::vector<float> time_array = animation_data.time_array;
	
	/////////////////////////////////
	//translation frame lerp
	/////////////////////////////////
	glm::vec3 final_mesh_pos = glm::vec3(0.f);
	std::vector<glm::vec3> trans_array = animation_data.translation_anim_array;
	
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (current_animation_time <= new_t) {
			float lerp = 1.f - (new_t - current_animation_time) / (new_t - old_t);
			
			glm::vec3 old_pos = glm::vec3(trans_array[i].x, trans_array[i].y, trans_array[i].z);
			glm::vec3 new_pos = glm::vec3(trans_array[i + 1].x, trans_array[i + 1].y, trans_array[i + 1].z);
			
			final_mesh_pos = glm::mix(old_pos, new_pos, lerp);
			
			break;
		}
		
		if ( (i == time_array.size() - 1) || current_animation_time >= time_array.back() )
			current_animation_time = 0.f;
	}
	return final_mesh_pos;
}

glm::quat Mesh::calculateCurrentRotation(const AnimationDataStruct& animation_data){
	/////////////////////////////////
	//rotation frame lerp
	/////////////////////////////////
	std::vector<float> time_array = animation_data.time_array;
	
	std::vector<glm::quat> rotations_vec = animation_data.rotation_anim_array;
	glm::quat final_mesh_rot = glm::quat(1.f, 0.f, 0.f, 0.f);
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (current_animation_time <= new_t) {
			float lerp = 1.f - (new_t - current_animation_time) / (new_t - old_t);
			
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
		
		if ( (i == time_array.size() - 1) || current_animation_time >= time_array.back() )
			current_animation_time = 0.f;
	}
	return final_mesh_rot;
}

glm::vec3 Mesh::calculateCurrentScale(const AnimationDataStruct& animation_data){
	/////////////////////////////////
	//scale frame lerp
	/////////////////////////////////
	std::vector<float> time_array = animation_data.time_array;
	
	glm::vec3 final_mesh_scale = glm::vec3(1.f);
	std::vector<glm::vec3> scale_array = animation_data.scale_anim_array;
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (current_animation_time <= new_t) {
			float lerp = 1.f - (new_t - current_animation_time) / (new_t - old_t);
			
			glm::vec3 old_scale = glm::vec3(scale_array[i].x, scale_array[i].y, scale_array[i].z);
			glm::vec3 new_scale = glm::vec3(scale_array[i + 1].x, scale_array[i + 1].y, scale_array[i + 1].z);
			
			final_mesh_scale = glm::mix(old_scale, new_scale, lerp);

			break;
		}
		
		if ( (i == time_array.size() - 1) || current_animation_time >= time_array.back() )
			current_animation_time = 0.f;
	}
	return final_mesh_scale;
}

void Mesh::updateSkinnedAnimation(){
		std::vector<std::size_t> bone_nodes_vec;//just to store bone/join indices for each matrix. Will be used to find parents for each bone
	
	if(!mesh_data.has_skin)
		return;
	
	bone_transform_matrix_array.clear();
	bone_skinned_matrix_array.clear();
	
	std::vector<AnimationDataStruct> bone_animations_vec = model->bone_animation_array;
	
	for (const AnimationDataStruct& bone_anim : bone_animations_vec) {
		glm::vec3 bone_pos = calculateCurrentTranslation(bone_anim);
		glm::quat bone_rot = calculateCurrentRotation(bone_anim);
		glm::vec3 bone_scale = calculateCurrentScale(bone_anim);
		
		
		if( model->getTinyGltfModel().nodes[bone_anim.node_index].name == "R_arm_024"){
//			if( bone_pos.y > 10 && bone_pos.z < -47 && bone_pos.z > -47.4 ){
//			}
//			bone_pos = glm::vec3(-2.45008, 9.14387, -48.5768);
//			std::cout << current_animation_time << " , " << std::flush;
//			printGlmVec3(bone_pos);
		}
		
		//create TRS for each bone
		glm::mat4 bone_transform = createTRSmatrix( bone_pos, bone_rot, bone_scale );
		
		//check if it is linked to another bone, and copy that root transform
		bone_nodes_vec.emplace_back(bone_anim.node_index);
		if (bone_anim.has_root) {
			auto bone_idx_itr = std::find(bone_nodes_vec.begin(), bone_nodes_vec.end(), bone_anim.root_idx);
			if ( bone_idx_itr != bone_nodes_vec.end() ) {
				std::size_t root_mat_index = std::distance(bone_nodes_vec.begin(), bone_idx_itr);
				bone_transform = bone_transform_matrix_array[ root_mat_index ] * bone_transform;
			}
		}
		

		
		
		bone_transform_matrix_array.emplace_back( bone_transform );
		
	}
	
	//FETCH AND SEND ALL inverseBindMatrices
	std::vector<glm::mat4> inverse_bind_mat_array = mesh_data.inverse_bind_matrix_array;
	
	for (std::size_t m{}; m < inverse_bind_mat_array.size(); m++) {
		
		glm::mat4 inverse_bind_mat(
			inverse_bind_mat_array[m][0].x, inverse_bind_mat_array[m][0].y, inverse_bind_mat_array[m][0].z, inverse_bind_mat_array[m][0].w,
			inverse_bind_mat_array[m][1].x, inverse_bind_mat_array[m][1].y, inverse_bind_mat_array[m][1].z, inverse_bind_mat_array[m][1].w,
			inverse_bind_mat_array[m][2].x, inverse_bind_mat_array[m][2].y, inverse_bind_mat_array[m][2].z, inverse_bind_mat_array[m][2].w,
			inverse_bind_mat_array[m][3].x, inverse_bind_mat_array[m][3].y, inverse_bind_mat_array[m][3].z, inverse_bind_mat_array[m][3].w
			);
		
		glm::mat4 skinned_mat = inverse_bind_mat * glm::transpose( bone_transform_matrix_array[m] );
		bone_skinned_matrix_array.emplace_back( glm::transpose( skinned_mat ) );
	}
	
	
}
