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
		std::cout << "WARNING: only 1 node is currently supported" << std::endl;
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
	unsigned char* uchar_array = (unsigned char*)model.buffers.front().data.data();
	
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
	if(!model.textures.empty()){
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
	}

	///////////////////////////
	//get translation/rot/scale [if any]
	///////////////////////////
	//warning, dont use .front()
	tinygltf::Node& node = model.nodes.front();
	if(!node.translation.empty())//translation
		translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
	if(!node.rotation.empty())//rotation
		rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
	if(!node.scale.empty())//scale
		scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
	
	
	///////////////////////////
	//ANIMATIONS
	///////////////////////////
	if(!model.animations.empty()){
		
		has_animation = true;
		
		//more than 1 animation?
		animation = model.animations.front();
		
		animation_name = animation.name;
		
		//bad idea, need more robust way of fetching time data 
		tinygltf::AnimationSampler& time_sampler = animation.samplers[0];
		
		int input_idx = time_sampler.input;
		
		AnimationDataStruct animation_data {};
		
		//fetch times
		{
			tinygltf::Accessor time_accessor = model.accessors[input_idx];
			int frame_count = time_accessor.count;
			
			int byteOffset = model.bufferViews[time_accessor.bufferView].byteOffset;
			int offset = byteOffset/sizeof(float);
			
			for(int t{}; t<frame_count; t++){
				float start_time = float_array[offset];
				float time_ = float_array[t + offset] - start_time; //subtract `start_time` to get it 0 initialized
				animation_data.time_array.emplace_back(time_);
			}
		}
		
		
		///////////////////////
		//fetch translations/scale/rots
		///////////////////////
		for(int i{}; i<animation.channels.size(); i++){
			tinygltf::AnimationChannel& channel = animation.channels[i];
			tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
			
			//ensure no parenting is used for now
			if(channel.target_node != 0)
				std::cout << ("Warning: Parent animations are not currently supported") << std::endl;
			
			int output_idx = sampler.output;
			
			std::string target_path = channel.target_path;
			tinygltf::Accessor& accessor = model.accessors[output_idx];
			int frame_count = model.accessors[output_idx].count;
			int byteOffset = model.bufferViews[accessor.bufferView].byteOffset;
			int offset = byteOffset/getSizeOfComponentType(accessor.componentType);
			
			for(int i{}; i<frame_count; i++){
				
				//translations
				if(target_path == "translation"){
					float x = float_array[(i*3) + 0 + offset];
					float y = float_array[(i*3) + 1 + offset];
					float z = float_array[(i*3) + 2 + offset];
					animation_data.translation_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "translation data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				}
				
				//rotations
				if(target_path == "rotation"){
					float x = float_array[(i*4) + 0 + offset];
					float y = float_array[(i*4) + 1 + offset];
					float z = float_array[(i*4) + 2 + offset];
					float w = float_array[(i*4) + 3 + offset];
					animation_data.rotation_anim_array.emplace_back( glm::quat(w, x, y, z) );
					std::cout << "rotation data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
				}
				
				//scale
				if(target_path == "scale"){
					float x = float_array[(i*3) + 0 + offset];
					float y = float_array[(i*3) + 1 + offset];
					float z = float_array[(i*3) + 2 + offset];
					animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				}
				
			}
			
		}
		
		animation_map.emplace_back(animation_data);
		
		//ensure all of time/scale/rot/pos arrays are of equal length
		if( translation_anim_array.size() != rotation_anim_array.size() || translation_anim_array.size() != scale_anim_array.size() || rotation_anim_array.size() != scale_anim_array.size() )
			throw std::logic_error("Translation, scale and rotation animation durations must be equal.");
		
		
		
	}
	
	/////////////////////
	//SKINS/JOINTS/WEIGHTS
	/////////////////////
	if(!model.skins.empty()){
		
		has_skin = true;
		
		if(model.skins.size() > 1)
			throw std::logic_error("Only 1 skin is currently supported");
		
		tinygltf::Skin& skin = model.skins.front();//.front()
		
		int joints_idx = primitive.attributes["JOINTS_0"];
		int weights_idx = primitive.attributes["WEIGHTS_0"];
		int skin_idx = skin.inverseBindMatrices;
		
		tinygltf::Accessor& joints_accessor = model.accessors[joints_idx];
		tinygltf::Accessor& weights_accessor = model.accessors[weights_idx];
		tinygltf::Accessor& skin_accessor = model.accessors[skin_idx];
		
		
		
		//joints
		{
			int byteOffset = model.bufferViews[joints_accessor.bufferView].byteOffset;
			int offset = byteOffset/getSizeOfComponentType(joints_accessor.componentType);
			
			int count = joints_accessor.count;
			
			for(std::size_t i{}; i<count; i++){
				unsigned char x = uchar_array[(i*4) + 0 + offset];
				unsigned char y = uchar_array[(i*4) + 1 + offset];
				unsigned char z = uchar_array[(i*4) + 2 + offset];
				unsigned char w = uchar_array[(i*4) + 3 + offset];
//				std::cout << "joint data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
				joints_array.emplace_back( glm::vec4(x, y, z, w) );
			}
		}
		
		
		//weights
		{
			int byteOffset = model.bufferViews[weights_accessor.bufferView].byteOffset;
			int offset = byteOffset/getSizeOfComponentType(weights_accessor.componentType);
			
			int count = weights_accessor.count;
			
			for(std::size_t i{}; i<count; i++){
				float x = float_array[(i*4) + 0 + offset];
				float y = float_array[(i*4) + 1 + offset];
				float z = float_array[(i*4) + 2 + offset];
				float w = float_array[(i*4) + 3 + offset];
				weights_array.emplace_back( glm::vec4(x, y, z, w) );
			}
		}
		
		
		//inverse bind matrices [i.e. the bones/their transformation matrices]
		{
			int byteOffset = model.bufferViews[skin_accessor.bufferView].byteOffset;
			int offset = byteOffset/getSizeOfComponentType(skin_accessor.componentType);
			
			int bone_count = skin.joints.size();
			
			glm::mat4 inv_bind_mat = glm::mat4(1.f); 
			
			for(int i{}; i<bone_count; i++){
				float x0_ = float_array[(i*16) + 0 + offset];
				float y0_ = float_array[(i*16) + 4 + offset];
				float z0_ = float_array[(i*16) + 8 + offset];
				float w0_ = float_array[(i*16) + 12 + offset];
				
				float x1_ = float_array[(i*16) + 1 + offset];
				float y1_ = float_array[(i*16) + 5 + offset];
				float z1_ = float_array[(i*16) + 9 + offset];
				float w1_ = float_array[(i*16) + 13 + offset];
				
				float x2_ = float_array[(i*16) + 2 + offset];
				float y2_ = float_array[(i*16) + 6 + offset];
				float z2_ = float_array[(i*16) + 10 + offset];
				float w2_ = float_array[(i*16) + 14 + offset];
				
				float x3_ = float_array[(i*16) + 3 + offset];
				float y3_ = float_array[(i*16) + 7 + offset];
				float z3_ = float_array[(i*16) + 11 + offset];
				float w3_ = float_array[(i*16) + 15 + offset];
				
				/*
				std::cout << "###############################" << std::endl;
				std::cout << "joint data [x: " << x0_ << ", y: " << y0_ << ", z: " << z0_ << ", w: " << w0_ << "]" << std::endl;
				std::cout << "joint data [x: " << x1_ << ", y: " << y1_ << ", z: " << z1_ << ", w: " << w1_ << "]" << std::endl;
				std::cout << "joint data [x: " << x2_ << ", y: " << y2_ << ", z: " << z2_ << ", w: " << w2_ << "]" << std::endl;
				std::cout << "joint data [x: " << x3_ << ", y: " << y3_ << ", z: " << z3_ << ", w: " << w3_ << "]" << std::endl;
				*/
				
				inv_bind_mat[0].x = x0_;
				inv_bind_mat[0].y = y0_;
				inv_bind_mat[0].z = z0_;
				inv_bind_mat[0].w = w0_;
				
				inv_bind_mat[1].x = x1_;
				inv_bind_mat[1].y = y1_;
				inv_bind_mat[1].z = z1_;
				inv_bind_mat[1].w = w1_;
				
				inv_bind_mat[2].x = x2_;
				inv_bind_mat[2].y = y2_;
				inv_bind_mat[2].z = z2_;
				inv_bind_mat[2].w = w2_;

				inv_bind_mat[3].x = x3_;
				inv_bind_mat[3].y = y3_;
				inv_bind_mat[3].z = z3_;
				inv_bind_mat[3].w = w3_;

				inverse_bind_matrix_array.emplace_back(inv_bind_mat);
				
			}
			
			
		}
		
		
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
