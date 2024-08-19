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
	
}


Mesh::~Mesh(){
	//delete VAO, VBO, EBO to avoid mem leaks
	vao.free();
	vbo.free();
	ebo.free();
	
	std::cout << "Mesh object destroyed" << std::endl;
}

void Mesh::update(){
	
	glBindVertexArray(vao.vao);
	
	//send camera and model matrices
	shader->use();
	shader->setMat4("modelMatrix", glm::mat4(1.f));
	shader->setMat4("viewMatrix", camera->GetViewMatrix());
	shader->setMat4("projMatrix", glm::perspective(camera->FovRads, window_manager->getAspectRatio(), 0.1f, 1000.0f));
	
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
	
	glDrawElements(GL_TRIANGLES, tri_indices.size(), GL_UNSIGNED_INT, 0);//rendering part
	
	//checks for rendering errors
	__GL_ERROR_THROW__("Failed to render Mesh.");
}
