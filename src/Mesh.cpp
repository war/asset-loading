#include "Mesh.h"


Mesh::Mesh(Camera* cam, ModelLoader* model_loader, MeshDataStruct _mesh_data, Shader* _shader, WindowManager* win_manager) : camera(cam), model(model_loader), mesh_data(_mesh_data), shader(_shader), window_manager(win_manager){
	
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
	
	//if skinned, update attribute and layouts
	if(mesh_data.has_skin){
		//setup pointers to the vertex position data `layout (location = 0)`
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16*sizeof(float), static_cast<void*>(0));
		glEnableVertexAttribArray(0);
		
		//setup pointers to the vertex UV coord data `layout (location = 1)`
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16*sizeof(float), reinterpret_cast<void*>( 3*sizeof(float) ));
		glEnableVertexAttribArray(1);
		
		//setup pointers to the vertex normal data `layout (location = 2)`
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 16*sizeof(float), reinterpret_cast<void*>( 5*sizeof(float) ));
		glEnableVertexAttribArray(2);
		
		//setup pointers to the JOINTS_0 data `layout (location = 3)` 
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), reinterpret_cast<void*>( 8*sizeof(float) ));
		glEnableVertexAttribArray(3);
		
		//setup pointers to the WEIGHTS_0 data `layout (location = 4)` 
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), reinterpret_cast<void*>( 12*sizeof(float) ));
		glEnableVertexAttribArray(4);	
	}
	//if normal/no skinning
	else{
		//setup pointers to the vertex position data `layout (location = 0)`
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), static_cast<void*>(0));
		glEnableVertexAttribArray(0);
		
		//setup pointers to the vertex UV coord data `layout (location = 1)`
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), reinterpret_cast<void*>( 3*sizeof(float) ));
		glEnableVertexAttribArray(1);
		
		//setup pointers to the vertex normal data `layout (location = 2)`
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), reinterpret_cast<void*>( 5*sizeof(float) ));
		glEnableVertexAttribArray(2);
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
	
	PRINT_COLOR( "Mesh object destroyed", 40, 220, 90);
}

void Mesh::update(){
	
	//update animations
	updateAnimation();
	
	//bind VAO
	glBindVertexArray(vao.vao);
	
	//apply translation
	modelMatrix = glm::translate(glm::mat4(1.f), position);

	//apply rotation
	glm::mat4 rotate_mat = glm::mat4(rotation);
	
	modelMatrix = glm::translate(glm::mat4(1.f), position) * glm::mat4(rotation);

	//finally apply scale
	modelMatrix = glm::scale(modelMatrix, scale);
	
	//send camera and model matrices
	shader->use();
	shader->setMat4("modelMatrix", modelMatrix);
	shader->setMat4("viewMatrix", camera->GetViewMatrix());
	shader->setMat4("projMatrix", glm::perspective(camera->FovRads, window_manager->getAspectRatio(), 0.1f, 1000.0f));
	
	///////////
	//textures
	///////////
	//diffuse tex
	if(model->has_diffuse_tex != 0){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model->getDiffuseTexture());
		shader->setInt("diffuse_tex", 0);//send Image to frag shader
	}
	
	//normal tex
	if(model->has_normal_tex != 0){
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, model->getNormalTexture());
		shader->setInt("normal_tex", 1);//send Image to frag shader
	}
	
	//metal tex
	if(model->has_metal_tex != 0){
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, model->getMetalTexture());
		shader->setInt("metal_tex", 2);//send Image to frag shader
	}
	
	//////////
	//skinning
	//////////
	//send skinned matrices to shader
	for(short m{}; m<m_boneSkinnedMatrices.size(); m++){
		std::string uniform_name = std::string("inverseBindMatrixArray[") + std::to_string(m) + std::string("].matrix");
		shader->setMat4(uniform_name.c_str(), m_boneSkinnedMatrices[m]);
	}
	
	//render triangles
	glDrawElements(GL_TRIANGLES, tri_indices.size(), GL_UNSIGNED_INT, 0);//rendering part
	
	//checks for rendering errors
	__GL_ERROR_THROW__("Failed to render Mesh.");
}


void Mesh::updateAnimation(){
	
	//quit if no animations for this mesh
	if(!mesh_data.animation_data.has_animation){
//		PRINT_WARN("no anim") ;
		return;
	}
	
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
	
	AnimationDataStruct& animation_data = mesh_data.animation_data;
	
	//ensure TRS anims are of equal lengths
	if( animation_data.translation_anim_array.size() != animation_data.rotation_anim_array.size() || animation_data.translation_anim_array.size() != animation_data.scale_anim_array.size() || animation_data.rotation_anim_array.size() != animation_data.scale_anim_array.size() )
		PRINT_WARN("Translation, scale and rotation animation durations must be equal.");
	
	//apply lerping between frame to set current pos/rot/scale
	position = calculateCurrentTranslation(animation_data);
	rotation = calculateCurrentRotation(animation_data);
	scale = calculateCurrentScale(animation_data);
	
//	std::cout << animation_data.translation_anim_array.size() << std::endl;
		
	
	updateSkinnedAnimation();
	
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
			
			//[EDGE CASE] CHECK IF NAN
			if ( std::isnan(final_mesh_pos.x) || std::isnan(final_mesh_pos.y) || std::isnan(final_mesh_pos.z) )
				final_mesh_pos = new_pos;
			
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
			if( quat_dot > 0.99f ){
				glm::vec4 old_r = glm::vec4(old_rot.x, old_rot.y, old_rot.z, old_rot.w);
				glm::vec4 new_r = glm::vec4(new_rot.x, new_rot.y, new_rot.z, new_rot.w);
				glm::vec4 lerped_r = glm::mix(old_r, new_r, lerp);
				
				final_mesh_rot = glm::quat(lerped_r.w, lerped_r.x, lerped_r.y, lerped_r.z);//overrites rot 
			}
			
			
			//EDGE CASE [if NaN. Seems to occur when old_rot and new_rot are very close, yielding NaN when Slerp is called]
			if ( std::isnan(final_mesh_rot.x) || std::isnan(final_mesh_rot.y) || std::isnan(final_mesh_rot.z) || std::isnan(final_mesh_rot.w) )
				final_mesh_rot = new_rot;
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
			
			//[EDGE CASE] CHECK IF NAN
			if ( std::isnan(final_mesh_scale.x) || std::isnan(final_mesh_scale.y) || std::isnan(final_mesh_scale.z) )
				final_mesh_scale = new_scale;
			
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
	
	m_boneTransformMatrices.clear();
	m_boneSkinnedMatrices.clear();
	
	std::vector<AnimationDataStruct> bone_animations_vec = model->animation_map;
	
	for (const AnimationDataStruct& bone_anim : bone_animations_vec) {
		glm::vec3 bone_pos = calculateCurrentTranslation(bone_anim);
		glm::quat bone_rot = calculateCurrentRotation(bone_anim);
		glm::vec3 bone_scale = calculateCurrentScale(bone_anim);
		
		//create TRS for each bone
		glm::mat4 bone_transform = createTRSmatrix( bone_pos, bone_rot, bone_scale );
		
		
		//TODO - ADD PARENTING
		//TODO - ADD PARENTING
		//check if it is linked to another bone, and copy that root transform
		bone_nodes_vec.emplace_back(bone_anim.node_index);
		if (bone_anim.has_root) {
			auto bone_idx_itr = std::find(bone_nodes_vec.begin(), bone_nodes_vec.end(), bone_anim.root_idx);
			if ( bone_idx_itr != bone_nodes_vec.end() ) {
				std::size_t root_mat_index = std::distance(bone_nodes_vec.begin(), bone_idx_itr);
				bone_transform = m_boneTransformMatrices[ root_mat_index ] * bone_transform;
			}
		}
		
		
		m_boneTransformMatrices.emplace_back( bone_transform );
		
	}
	
	//FETCH AND SEND ALL inverseBindMatrices
	std::vector<glm::mat4> inverseBindMat_vec = mesh_data.inverse_bind_matrix_array;
	
	for (std::size_t m{}; m < inverseBindMat_vec.size(); m++) {
		
		glm::mat4 invBindMat(
			inverseBindMat_vec[m][0].x, inverseBindMat_vec[m][0].y, inverseBindMat_vec[m][0].z, inverseBindMat_vec[m][0].w,
			inverseBindMat_vec[m][1].x, inverseBindMat_vec[m][1].y, inverseBindMat_vec[m][1].z, inverseBindMat_vec[m][1].w,
			inverseBindMat_vec[m][2].x, inverseBindMat_vec[m][2].y, inverseBindMat_vec[m][2].z, inverseBindMat_vec[m][2].w,
			inverseBindMat_vec[m][3].x, inverseBindMat_vec[m][3].y, inverseBindMat_vec[m][3].z, inverseBindMat_vec[m][3].w
			);
		
		glm::mat4 skinnedMatrix = invBindMat * glm::transpose( m_boneTransformMatrices[m] );
		m_boneSkinnedMatrices.emplace_back( glm::transpose( skinnedMatrix ) );
	}

	
//	if(model->)
	
	
	
}
