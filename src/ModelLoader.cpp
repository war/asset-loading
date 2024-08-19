#include "ModelLoader.h"

ModelLoader::ModelLoader() {}
ModelLoader::~ModelLoader() {
	glDeleteTextures(1, &diffuse_texture);
	glDeleteTextures(1, &normal_texture);
	glDeleteTextures(1, &metal_texture);
}

bool ModelLoader::LoadModel(const std::string& path, const std::string& diffuse_tex_name, const std::string& normal_tex_name, const std::string& metallic_tex_name) {

	std::string error;
	std::string warning;
	
	bool load_resource = tiny_gltf.LoadASCIIFromFile(&model, &error, &warning, path);//also need to add support for binary glTF loading
	
	if(!load_resource){
		std::cout << "Failed to load model with path: `" + path + "`. Error: " + error + ". Warning: " + warning << std::endl;
		return false;
	}
	
	if(model.nodes.size() > 1)
		throw std::logic_error("Only 1 node is currently supported.");
	if(model.meshes.size() > 1)
		throw std::logic_error("Only 1 mesh is currently supported.");
	
	
	//warning, bad idea doing .front() [could be more than 1 mesh]
	mesh = model.meshes.front();
	mesh_name = mesh.name;
	
	//warning, bad idea doing .front()
	tinygltf::Primitive primitive = mesh.primitives.front();
	int vert_pos_idx = primitive.attributes["POSITION"];
	int vert_norm_idx = primitive.attributes["NORMAL"];
	int vert_uv_idx = primitive.attributes["TEXCOORD_0"];
	int vert_index_count_idx = primitive.indices;
	
	tinygltf::Accessor vert_pos_accessor = model.accessors[vert_pos_idx];
	tinygltf::Accessor vert_norm_accessor = model.accessors[vert_norm_idx];
	tinygltf::Accessor vert_uv_accessor = model.accessors[vert_uv_idx];
	tinygltf::Accessor vert_index_accessor = model.accessors[vert_index_count_idx];
	
	//warning, bad idea doing .front()
	//cast the buffers to 1D arrays of common data types (float, unsigned short, etc). These arrays hold the raw vertex positions, normals, uv's, animation & skinning data etc
	float* float_array = (float*)model.buffers.front().data.data();
	unsigned short* ushort_array = (unsigned short*)model.buffers.front().data.data();
	unsigned int* uint_array = (unsigned int*)model.buffers.front().data.data();
	
	/////////////////////
	//VERTEX DATA
	/////////////////////
	{
		int vertex_count = vert_pos_accessor.count;
		
		int byteOffset = model.bufferViews[vert_pos_accessor.bufferView].byteOffset;
		int offset = byteOffset/sizeof(float);
		
		//get vertex positions
		for(std::size_t i{}; i<vertex_count; i++){
			float x = float_array[(i*3) + 0 + offset];
			float y = float_array[(i*3) + 1 + offset];
			float z = float_array[(i*3) + 2 + offset];
			
			vertex_positions_array.emplace_back( glm::vec3(x, y, z) );
			
//			std::cout << "Vertex pos data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
		}
	}
	
	
	/////////////////////
	//NORMAL DATA
	/////////////////////
	{
		int normal_count = vert_norm_accessor.count;
		
		int byteOffset = model.bufferViews[vert_norm_accessor.bufferView].byteOffset;
		int offset = byteOffset/sizeof(float);
		
		//get vertex normals
		for(std::size_t i{}; i<normal_count; i++){
			float x = float_array[(i*3) + 0 + offset];
			float y = float_array[(i*3) + 1 + offset];
			float z = float_array[(i*3) + 2 + offset];
			
			vertex_normals_array.emplace_back( glm::vec3(x, y, z) );
			
//			std::cout << "Vertex normal data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
		}
	}
	
	
	/////////////////////
	//UVS
	/////////////////////
	{
		int uvs_count = vert_uv_accessor.count;
		
		int byteOffset = model.bufferViews[vert_uv_accessor.bufferView].byteOffset;
		int offset = byteOffset/getSizeOfComponentType(vert_uv_accessor.componentType);//REPLACE SIZEOF WITH REAL DATA TYPE
		
		//get vertex UV's
		for(std::size_t i{}; i<uvs_count; i++){
			float x = float_array[(i*2) + 0 + offset];
			float y = float_array[(i*2) + 1 + offset];
			
			vertex_uvs_array.emplace_back( glm::vec2(x, y) );
			
//			std::cout << "Vertex UV data [x: " << x << ", y: " << y << "]" << std::endl;
		}
	}
	
	
	/////////////////////
	//INDICES
	/////////////////////
	{
		int index_count = vert_index_accessor.count;
		
		int byteOffset = model.bufferViews[vert_index_accessor.bufferView].byteOffset;
		int offset = byteOffset/getSizeOfComponentType(vert_index_accessor.componentType);
		
		//get vertex indices
		for(std::size_t i{}; i<index_count; i++){
			
			if(vert_index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				vertex_indices_array.emplace_back( ushort_array[i + offset] );
			else if(vert_index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				vertex_indices_array.emplace_back( uint_array[i + offset] );
		}
	}
	
	/////////////////////
	//TEXTURES
	/////////////////////
//	if(model.textures.size() > 1){
//		throw std::logic_error("Only 1 texture is currently supported.");
//	}
		tinygltf::Texture tex = model.textures.front();//DIFFUSE TEX
//		tinygltf::Image diff_img = model.images[tex.source];//DIFFUSE TEX
		tinygltf::Image diff_img;//DIFFUSE TEX
		tinygltf::Image norm_img;//DIFFUSE TEX
		tinygltf::Image metal_img;//DIFFUSE TEX
		
	for(auto t : model.textures){
		if(model.images[t.source].name == diffuse_tex_name)
			diff_img = model.images[t.source];
		if(model.images[t.source].name == normal_tex_name)
			norm_img = model.images[t.source];
		if(model.images[t.source].name == metallic_tex_name)
			metal_img = model.images[t.source];
	}
		
	///////////
	//diffuse
	///////////
	if(diff_img.width != -1){
		has_diffuse_tex = true;
		
		glGenTextures(1, &diffuse_texture);
		//	glActiveTexture(GL_TEXTURE0 + m_slot); //Is setting active texture even needed?
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//from ex
		
		/* set texture filtering */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //far away texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //close up texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		/* fill data buffer */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diff_img.width, diff_img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, diff_img.image.data());
		
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, 0);//unbind once we've finished [by binding to 0]
		
		__GL_ERROR_THROW__("Texture creation failed"); //check for GL any errors
	}
	///////////
	//normal
	///////////
	if(norm_img.width != -1){
		has_normal_tex = true;
		
		glGenTextures(1, &normal_texture);
		//	glActiveTexture(GL_TEXTURE0 + m_slot); //Is setting active texture even needed?
		glBindTexture(GL_TEXTURE_2D, normal_texture);
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//from ex
		
		/* set texture filtering */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //far away texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //close up texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		/* fill data buffer */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, norm_img.width, norm_img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, norm_img.image.data());
		
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, 0);//unbind once we've finished [by binding to 0]
		
		__GL_ERROR_THROW__("Texture creation failed"); //check for GL any errors
	}
	///////////
	//metal
	///////////
	if(metal_img.width != -1){
		has_normal_tex = true;
		
		glGenTextures(1, &metal_texture);
		//	glActiveTexture(GL_TEXTURE0 + m_slot); //Is setting active texture even needed?
		glBindTexture(GL_TEXTURE_2D, metal_texture);
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//from ex
		
		/* set texture filtering */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //far away texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //close up texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		/* fill data buffer */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, metal_img.width, metal_img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, metal_img.image.data());
		
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, 0);//unbind once we've finished [by binding to 0]
		
		__GL_ERROR_THROW__("Texture creation failed"); //check for GL any errors
	}

	
	
    return true;
}

void ModelLoader::Render() {
     // test
}


long unsigned int ModelLoader::getSizeOfComponentType(int component_type){
	if(component_type == TINYGLTF_COMPONENT_TYPE_BYTE)
		return sizeof(char);
	if(component_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
		return sizeof(unsigned char);
	if(component_type == TINYGLTF_COMPONENT_TYPE_SHORT)
		return sizeof(short int);
	if(component_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
		return sizeof(unsigned short int);
	if(component_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
		return sizeof(unsigned int);
	if(component_type == TINYGLTF_COMPONENT_TYPE_INT)
		return sizeof(int);
	if(component_type == TINYGLTF_COMPONENT_TYPE_FLOAT)
		return sizeof(float);
	
	std::cout << "Warning: component data type is not supported by the glTF format. Returning 0." << std::endl;
	
	return 0;//returns 0 if none of the above types is supported by the glTF format [indicates an issue with the mesh]
}
