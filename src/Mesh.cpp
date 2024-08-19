#include "Mesh.h"


Mesh::Mesh(Camera* cam, ModelLoader* model_loader, Shader* _shader, WindowManager* win_manager) : camera(cam), model(model_loader), shader(_shader), window_manager(win_manager){
	
	//fill in the arrays with flattened GLfloat and GLuint vertex/index data
	for(unsigned int i{}; i<model->getVertexPositions().size(); i++){
		glm::vec3 vert_pos = model->getVertexPositions()[i];
		glm::vec2 vert_uv = model->getVertexUVs()[i];
		glm::vec3 vert_norm = model->getVertexNormals()[i];
		
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
	}
	for(unsigned int v_idx : model->getIndices()){
		tri_indices.emplace_back( (GLuint)v_idx );
	}

	vao.bind();
	vbo = VBO(tri_vertices);
	ebo = EBO(tri_indices);
	
	//setup pointers to the vertex position data `layout (location = 0)`
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), static_cast<void*>(0));
	glEnableVertexAttribArray(0);
	
	//setup pointers to the vertex UV coord data `layout (location = 1)`
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), reinterpret_cast<void*>( 3*sizeof(float) ));
	glEnableVertexAttribArray(1);
	
	//setup pointers to the vertex normal data `layout (location = 2)`
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), reinterpret_cast<void*>( 5*sizeof(float) ));
	glEnableVertexAttribArray(2);
	
	
	vbo.unbind();
	vao.unbind();
	ebo.unbind();
	
	//only throws error if it failed to fill in buffers
	__GL_ERROR_THROW__("Failed to generate Mesh object"); //check for any GL errors
	
	
	//set position/scale/orn
	position = model->getTranslation();
	rotation = model->getRotation();
	scale = model->getScale();
	
}


Mesh::~Mesh(){
	//delete VAO, VBO, EBO to avoid mem leaks
	vao.free();
	vbo.free();
	ebo.free();
	
	std::cout << "Mesh object destroyed" << std::endl;
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
	
	//render triangles
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
	current_animation_time += window_manager->GetDeltaTime();
	rot_curr_t += window_manager->GetDeltaTime();
	
	linearBlendAnimation();
	
}

void Mesh::linearBlendAnimation(){
	
	
	std::vector<float>& time_array = model->time_array;
	
	
	/////////////////////////////////
	//translation frame lerp
	/////////////////////////////////
	glm::vec3 final_mesh_pos = glm::vec3(0.f);
	std::vector<glm::vec3>& trans_array = model->translation_anim_array;
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
	position = final_mesh_pos;
	
	
	/////////////////////////////////
	//rotation frame lerp
	/////////////////////////////////
	std::vector<glm::quat>& rotations_vec = model->rotation_anim_array;
	glm::quat final_mesh_rot = glm::quat(1.f, 0.f, 0.f, 0.f);
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (current_animation_time <= new_t) {
			float lerp = 1.f - (new_t - current_animation_time) / (new_t - old_t);
			
			glm::quat old_rot = glm::quat(rotations_vec[i].w, rotations_vec[i].x, rotations_vec[i].y, rotations_vec[i].z);
			glm::quat new_rot = glm::quat(rotations_vec[i + 1].w, rotations_vec[i + 1].x, rotations_vec[i + 1].y, rotations_vec[i + 1].z);
			
			final_mesh_rot = glm::mix(old_rot, new_rot, lerp);
			
			//EDGE CASE [if NaN. Seems to occur when old_rot and new_rot are very close, yielding NaN when Slerp is called]
			if ( std::isnan(final_mesh_rot.x) || std::isnan(final_mesh_rot.y) || std::isnan(final_mesh_rot.z) || std::isnan(final_mesh_rot.w) )
				final_mesh_rot = new_rot;
			break;
		}
		
		if ( (i == time_array.size() - 1) || current_animation_time >= time_array.back() )
			current_animation_time = 0.f;
	}
	rotation = final_mesh_rot;
	
	
	
	/////////////////////////////////
	//scale frame lerp
	/////////////////////////////////
	glm::vec3 final_mesh_scale = glm::vec3(1.f);
	std::vector<glm::vec3>& scale_array = model->scale_anim_array;
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
	scale = final_mesh_scale;
	
	
	
	
	
}
