#include "ModelLoader.h"

ModelLoader::ModelLoader(const std::string& path, const std::string& diffuse_tex_name, const std::string& normal_tex_name, const std::string& metallic_tex_name) : diffuse_texture_name(diffuse_tex_name), normal_texture_name(normal_tex_name), metallic_texture_name(metallic_tex_name) {
	
	std::string error;
	std::string warning;
	
	bool load_resource = tiny_gltf.LoadASCIIFromFile(&model, &error, &warning, path);//also need to add support for binary glTF loading
	
	if(!load_resource){
		std::cout << "Failed to load model with path: `" + path + "`. Error: " + error + ". Warning: " + warning << std::endl;
		return;
	}
	
	if(model.nodes.size() > 1)
		PRINT_WARN("only 1 node is currently supported");
//	if(model.meshes.size() > 1)
//		throw std::logic_error("Only 1 mesh is currently supported.");
	if(model.buffers.size() > 1)
		throw std::logic_error("Only 1 buffer is currently supported.");
	if(model.buffers.empty())
		throw std::logic_error("Buffer must not be empty.");
	
	//cast the buffers to 1D arrays of common data types (float, unsigned short, etc). These arrays hold the raw vertex positions, normals, uv's, animation & skinning data etc
	float_array = (float*)model.buffers.front().data.data();
	ushort_array = (unsigned short*)model.buffers.front().data.data();
	uint_array = (unsigned int*)model.buffers.front().data.data();
	uchar_array = (unsigned char*)model.buffers.front().data.data();
	
	//	loadEmpties();
	
	/////////////////
	//LOAD ALL MESHES
	/////////////////
	for(int m{}; m<model.meshes.size(); m++){
		tinygltf::Mesh& mesh = model.meshes[m];
		
		MeshDataStruct mesh_data_struct {};//will store all mesh data [vertex pos/norm/uv, global mesh pos/rot etc]
		
		mesh_data_struct.name	= mesh.name;
		
		if(mesh.primitives.size() > 1)
			throw std::logic_error("Only 1 primitive per mesh is supported");
		
		/*
		//legacy code
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
		*/
		
		
		/////////////////////
		//VERTEX DATA
		/////////////////////
		mesh_data_struct.vertex_positions_array = getVertexPositions(mesh);
		
		///////////////////////
		//NORMAL DATA
		/////////////////////
		mesh_data_struct.vertex_normals_array = getVertexNormals(mesh);
		
		///////////////////////
		//UVS
		/////////////////////
		mesh_data_struct.vertex_uvs_array = getVertexUVs(mesh);
		
		///////////////////////
		//INDICES
		/////////////////////
		mesh_data_struct.vertex_indices_array = getIndices(mesh);
		
		///////////////////////////
		//GLOBAL POS/ROT/SCALE
		///////////////////////////
		{
			int mesh_node_idx = getMeshNodeIndex(mesh);
			tinygltf::Node& node = model.nodes[mesh_node_idx];
			//find node this mesh is assigned to
			mesh_data_struct.node_index = mesh_node_idx;
			if(!node.translation.empty())//translation
				mesh_data_struct.translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
			if(!node.rotation.empty())//rotation
				mesh_data_struct.rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			if(!node.scale.empty())//scale
				mesh_data_struct.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
		}
		
		///////////////////////////
		//SKINNING
		///////////////////////////
		if(!model.skins.empty()){//need better checks than this
			
			has_skin = true;//USELESS --REMOVE THIS once animation for skinning reworked [needed rn as used by animations]
			
			mesh_data_struct.has_skin = true;
			
			//joints
			mesh_data_struct.joints_array = getSkinJoints(mesh);
			
			//weights
			mesh_data_struct.weights_array = getSkinWeights(mesh);
		
			//inverse bind matrices
			mesh_data_struct.inverse_bind_matrix_array = getInverseBindMatrices(mesh);
		}
			
		
		///////////////////////////
		//ANIMATIONS
		///////////////////////////
		mesh_data_struct.animation_data = getMeshAnimationData(mesh);
		
		//add to MeshDataStruct array
		mesh_data_struct_array.emplace_back(mesh_data_struct);
	}


	
	
	
	///////////////
	//TEXTURES
	///////////////
	generateTextures();
	

	/*
	/////////////////////
	//SKINS/JOINTS/WEIGHTS
	/////////////////////
	if(!model.skins.empty()){
		
		
		if(model.skins.size() > 1)
			throw std::logic_error("Only 1 skin is currently supported");
		
		has_skin = true;
		
		skin = model.skins.front();//.front()
		
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
				
//				std::cout << "###############################" << std::endl;
//				std::cout << "joint data [x: " << x0_ << ", y: " << y0_ << ", z: " << z0_ << ", w: " << w0_ << "]" << std::endl;
//				std::cout << "joint data [x: " << x1_ << ", y: " << y1_ << ", z: " << z1_ << ", w: " << w1_ << "]" << std::endl;
//				std::cout << "joint data [x: " << x2_ << ", y: " << y2_ << ", z: " << z2_ << ", w: " << w2_ << "]" << std::endl;
//				std::cout << "joint data [x: " << x3_ << ", y: " << y3_ << ", z: " << z3_ << ", w: " << w3_ << "]" << std::endl;
				
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
	*/
	
	
	
	///////////////////////////
	//ANIMATIONS
	///////////////////////////
	if(!model.animations.empty()){
		
		has_animation = true;
		
		//more than 1 animation?
		animation_tinygltf = model.animations.front();
		
		animation_name = animation_tinygltf.name;
		
		static std::size_t idx {};//used to keep track of individual bones
		for(int c{}; c<animation_tinygltf.channels.size(); c++){
			
			//bad idea, need more robust way of fetching time data 
			tinygltf::AnimationSampler& time_sampler = animation_tinygltf.samplers[0];
			
			int input_idx = time_sampler.input;
			
			/////////////////////
			//fetch times
			/////////////////////
			std::vector<float> times;
			{
				tinygltf::Accessor time_accessor = model.accessors[input_idx];
				int frame_count = time_accessor.count;
				
				int byteOffset = model.bufferViews[time_accessor.bufferView].byteOffset;
				int offset = byteOffset/sizeof(float);
				
				for(int t{}; t<frame_count; t++){
					float start_time = float_array[offset];
					float time_ = float_array[t + offset] - start_time; //subtract `start_time` to get it 0 initialized
					times.emplace_back(time_);
				}
			}
			
			///////////////////////
			//fetch translations/rots/scale
			///////////////////////
			tinygltf::AnimationChannel& channel = animation_tinygltf.channels[c];
			tinygltf::AnimationSampler& sampler = animation_tinygltf.samplers[channel.sampler];
			
			//the node it belongs to
			int node_idx = channel.target_node;
			
			
			///////////////////
			///////ADDING TO ARRAY
			if(idx != node_idx){
				animation_map.emplace_back(AnimationDataStruct{}); 
				idx = node_idx;
			}
			if(animation_map.empty())
				animation_map.emplace_back(AnimationDataStruct{}); 
			
			AnimationDataStruct& animation_data = animation_map.back();
			
			animation_data.time_array = times;
			
			animation_data.has_animation = true;
			animation_data.node_index = node_idx;
			/////////////////////////////////////
			/////////////////////////////////////
			
			
			///////////////
			///////////////
			///////////////
			//FIND ROOT BONE
			if(has_skin){
				for(std::size_t n{}; n<model.nodes.size(); n++){
					std::vector<int> childs_vec = model.nodes[n].children;
					
					if(childs_vec.size()>1)
						PRINT_WARN("IMPROVE SYSTEM FOR MORE THAN 1 CHILD");
					
					if( std::find(childs_vec.begin(), childs_vec.end(), node_idx) != childs_vec.end() ){
						animation_data.has_root = true;
						animation_data.root_idx = n;
						break;
					}
					
				}
			}
			///////////////////////////
			///////////////////////////
			///////////////////////////
			
			
			
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
//					std::cout << "rotation data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
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
		
		
	}
	/*
	*/
	
}

ModelLoader::~ModelLoader() {
	glDeleteTextures(1, &diffuse_texture);
	glDeleteTextures(1, &normal_texture);
	glDeleteTextures(1, &metal_texture);
	
	PRINT_COLOR( "ModelLoader object destroyed", 40, 220, 90);
}

/*
void ModelLoader::Render() {
     // test
}
*/

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

std::vector<glm::vec3> ModelLoader::getVertexPositions(const tinygltf::Mesh& mesh){
	std::vector<glm::vec3> vert_pos_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	int vert_pos_idx = primitive.attributes["POSITION"];
	
	tinygltf::Accessor vert_pos_accessor = model.accessors[vert_pos_idx];

	int vertex_count = vert_pos_accessor.count;
	int byteOffset = model.bufferViews[vert_pos_accessor.bufferView].byteOffset;
	int offset = byteOffset/sizeof(float);
	
	//get vertex positions
	for(std::size_t i{}; i<vertex_count; i++){
		float x = float_array[(i*3) + 0 + offset];
		float y = float_array[(i*3) + 1 + offset];
		float z = float_array[(i*3) + 2 + offset];
		
		vert_pos_array.emplace_back( glm::vec3(x, y, z) );
//			std::cout << "Vertex pos data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
	}
	return vert_pos_array;
}

std::vector<glm::vec3> ModelLoader::getVertexNormals(const tinygltf::Mesh& mesh){

	std::vector<glm::vec3> vert_norm_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	int vert_norm_idx = primitive.attributes["NORMAL"];
	
	tinygltf::Accessor vert_norm_accessor = model.accessors[vert_norm_idx];
	
	int normal_count = vert_norm_accessor.count;
	int byteOffset = model.bufferViews[vert_norm_accessor.bufferView].byteOffset;
	int offset = byteOffset/sizeof(float);
	
	//get vertex normals
	for(std::size_t i{}; i<normal_count; i++){
		float x = float_array[(i*3) + 0 + offset];
		float y = float_array[(i*3) + 1 + offset];
		float z = float_array[(i*3) + 2 + offset];
		vert_norm_array.emplace_back( glm::vec3(x, y, z) );
//			std::cout << "Vertex normal data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
	}
	
	return vert_norm_array;
}

std::vector<glm::vec2> ModelLoader::getVertexUVs(const tinygltf::Mesh& mesh){
	
	std::vector<glm::vec2> vert_uvs_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	int vert_uv_idx = primitive.attributes["TEXCOORD_0"];
	
	tinygltf::Accessor vert_uv_accessor = model.accessors[vert_uv_idx];
	
	int uvs_count = vert_uv_accessor.count;
	int byteOffset = model.bufferViews[vert_uv_accessor.bufferView].byteOffset;
	int offset = byteOffset/getSizeOfComponentType(vert_uv_accessor.componentType);//REPLACE SIZEOF WITH REAL DATA TYPE
	
	//get vertex UV's
	for(std::size_t i{}; i<uvs_count; i++){
		float x = float_array[(i*2) + 0 + offset];
		float y = float_array[(i*2) + 1 + offset];
		vert_uvs_array.emplace_back( glm::vec2(x, y) );
//			std::cout << "Vertex UV data [x: " << x << ", y: " << y << "]" << std::endl;
	}
	
	return vert_uvs_array;
}

std::vector<unsigned int> ModelLoader::getIndices(const tinygltf::Mesh& mesh){
	
	std::vector<unsigned int> indices_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	int vert_index_count_idx = primitive.indices;
	
	tinygltf::Accessor vert_index_accessor = model.accessors[vert_index_count_idx];
	
	int index_count = vert_index_accessor.count;
	int byteOffset = model.bufferViews[vert_index_accessor.bufferView].byteOffset;
	int offset = byteOffset/getSizeOfComponentType(vert_index_accessor.componentType);
	
	//get vertex indices
	for(std::size_t i{}; i<index_count; i++){
		if(vert_index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			indices_array.emplace_back( ushort_array[i + offset] );
		else if(vert_index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			indices_array.emplace_back( uint_array[i + offset] );
	}
	
	return indices_array;
}

void ModelLoader::generateTextures(){
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
			if(model.images[t.source].name == diffuse_texture_name)
				diff_img = model.images[t.source];
			if(model.images[t.source].name == normal_texture_name)
				norm_img = model.images[t.source];
			if(model.images[t.source].name == metallic_texture_name)
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
}

int ModelLoader::getMeshNodeIndex(const tinygltf::Mesh& mesh){
	//finds node index for given mesh
	
	int index = -1;
	
	for(int m{}; m<model.meshes.size(); m++){
		
		if(model.meshes[m] != mesh)
			continue;
		
		for(int n{}; n<model.nodes.size(); n++){
			tinygltf::Node& node = model.nodes[n];
			//find node this mesh is assigned to
			if(node.mesh == m){
				return m;
			}
		}
		
	}
	
	return index;
	
}

AnimationDataStruct ModelLoader::getMeshAnimationData(const tinygltf::Mesh& mesh){
	
	AnimationDataStruct animation_data;
	
	///////////////////////////
	//ANIMATIONS
	///////////////////////////
	if(model.animations.empty()){
		return animation_data;
	}
		
	tinygltf::Animation animation;
	
	//find the tinygltf::Animation object for this mesh
	bool animations_found = false;
	for(const tinygltf::Animation& anim : model.animations){
		for(const tinygltf::AnimationChannel& chan : anim.channels){
			if(chan.target_node == getMeshNodeIndex(mesh)){
				animation = anim;
				animations_found = true;
				std::cout << mesh.name << ", " << getMeshNodeIndex(mesh) << std::endl;
				break;
			}
		}
	}
	
	//if no animations for the current mesh, break out
	if(!animations_found){
		PRINT_WARN("no anims found for " + mesh.name);
		return animation_data;
	}
	
	for(int c{}; c<animation.channels.size(); c++){
		
		//bad idea, need more robust way of fetching time data 
		tinygltf::AnimationSampler& time_sampler = animation.samplers[0];
		
		int input_idx = time_sampler.input;
		
		/////////////////////
		//fetch times
		/////////////////////
		std::vector<float> times;
		{
			tinygltf::Accessor time_accessor = model.accessors[input_idx];
			int frame_count = time_accessor.count;
			
			int byteOffset = model.bufferViews[time_accessor.bufferView].byteOffset;
			int offset = byteOffset/sizeof(float);
			
			for(int t{}; t<frame_count; t++){
				float start_time = float_array[offset];
				float time_ = float_array[t + offset] - start_time; //subtract `start_time` to get it 0 initialized
				times.emplace_back(time_);
			}
		}
		animation_data.time_array = times;
		
		animation_data.has_animation = true;
		
		tinygltf::AnimationChannel& channel = animation.channels[c];
		tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
		
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
//					std::cout << "rotation data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
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
	
	
	
	
	
	
	return animation_data;
}

std::vector<glm::vec4> ModelLoader::getSkinJoints(const tinygltf::Mesh& mesh){
	std::vector<glm::vec4> joints_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	int joints_idx = primitive.attributes["JOINTS_0"];
	
	tinygltf::Accessor& joints_accessor = model.accessors[joints_idx];
	
	//joints
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
	
	return joints_array;
}

std::vector<glm::vec4> ModelLoader::getSkinWeights(const tinygltf::Mesh& mesh){
	
	std::vector<glm::vec4> weights_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	int weights_idx = primitive.attributes["WEIGHTS_0"];
	
	tinygltf::Accessor& weights_accessor = model.accessors[weights_idx];
	
	//weights
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
	
	return weights_array;
}

std::vector<glm::mat4> ModelLoader::getInverseBindMatrices(const tinygltf::Mesh& mesh){
	
	std::vector<glm::mat4> inverse_bind_matrix_array;
	
	if(model.skins.size() > 1)
		throw std::logic_error("Only 1 skin is currently supported");
	
	skin = model.skins.front();//.front()
	
	int skin_idx = skin.inverseBindMatrices;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	tinygltf::Accessor& skin_accessor = model.accessors[skin_idx];
	
	//inverse bind matrices [i.e. the bones/their transformation matrices]
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
		
//				std::cout << "###############################" << std::endl;
//				std::cout << "joint data [x: " << x0_ << ", y: " << y0_ << ", z: " << z0_ << ", w: " << w0_ << "]" << std::endl;
//				std::cout << "joint data [x: " << x1_ << ", y: " << y1_ << ", z: " << z1_ << ", w: " << w1_ << "]" << std::endl;
//				std::cout << "joint data [x: " << x2_ << ", y: " << y2_ << ", z: " << z2_ << ", w: " << w2_ << "]" << std::endl;
//				std::cout << "joint data [x: " << x3_ << ", y: " << y3_ << ", z: " << z3_ << ", w: " << w3_ << "]" << std::endl;
		
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
		
	
	return inverse_bind_matrix_array;
}

void ModelLoader::loadEmpties(){
	//warning, bad idea doing .front()
	
	//filter meshes from non meshes
	for(int n{}; n<model.nodes.size(); n++){
		
		const tinygltf::Node& node = model.nodes[n];

		int node_index = n;
		
		//if NOT mesh
		if(node.mesh == -1){
			Empty empty;
			
			///////////////////////////
			//ANIMATIONS
			///////////////////////////
			tinygltf::Animation animation;
			
			////////////////////////////////////
			//check to seee if anim exists for this node
			for(const tinygltf::Animation& a : model.animations){
				int target_idx  {};
				
				for(tinygltf::AnimationChannel ch : a.channels){
					if(ch.target_node == node_index){
						empty.has_animation = true;
						std::cout << node_index << std::endl;
						break;
					}
				}
				if(empty.has_animation){
					std::cout << a.name << std::endl;
					animation = a;
					break;
				}
			}
			////////////////////////////////////
			
			//quit if no anims
			if(!empty.has_animation)
				return;
			
			empty.animation_name = animation.name;

//				static std::size_t idx {};//used to keep track of individual bones
			for(int c{}; c<animation.channels.size(); c++){
				
				tinygltf::AnimationChannel& channel = animation.channels[c];
				tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
				
				//skip this sampler if not for current node
				if(channel.target_node != node_index)
					continue;
				
				//bad idea, need more robust way of fetching time data 
				tinygltf::AnimationSampler& time_sampler = animation.samplers[0];
				
				int input_idx = time_sampler.input;
				
				/////////////////////
				//fetch times
				/////////////////////
				std::vector<float> times;
				{
					tinygltf::Accessor time_accessor = model.accessors[input_idx];
					int frame_count = time_accessor.count;
					
					int byteOffset = model.bufferViews[time_accessor.bufferView].byteOffset;
					int offset = byteOffset/sizeof(float);
					
					for(int t{}; t<frame_count; t++){
						float start_time = float_array[offset];
						float time_ = float_array[t + offset] - start_time; //subtract `start_time` to get it 0 initialized
						times.emplace_back(time_);
					}
				}
				empty.animation_data.time_array = times;

				///////////////////////
				//fetch translations/rots/scale
				///////////////////////
				
//					///////////////////
//					///////ADDING TO ARRAY
//					if(idx != node_idx){
//						animation_map.emplace_back(AnimationDataStruct{}); 
//						idx = node_idx;
//					}
//					if(animation_map.empty())
//						animation_map.emplace_back(AnimationDataStruct{}); 
//					
//					AnimationDataStruct& animation_data = animation_map.back();
//					
//					animation_data.time_array = times;
//					
//					animation_data.has_animation = true;
//					animation_data.node_index = node_idx;
//					/////////////////////////////////////
//					/////////////////////////////////////
//					
//
//					
//					
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
						empty.animation_data.translation_anim_array.emplace_back( glm::vec3(x, y, z) );
//						std::cout << "translation data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
					}
					
					//rotations
					if(target_path == "rotation"){
						float x = float_array[(i*4) + 0 + offset];
						float y = float_array[(i*4) + 1 + offset];
						float z = float_array[(i*4) + 2 + offset];
						float w = float_array[(i*4) + 3 + offset];
						empty.animation_data.rotation_anim_array.emplace_back( glm::quat(w, x, y, z) );
	//					std::cout << "rotation data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
					}
					
					//scale
					if(target_path == "scale"){
						float x = float_array[(i*3) + 0 + offset];
						float y = float_array[(i*3) + 1 + offset];
						float z = float_array[(i*3) + 2 + offset];
						empty.animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
	//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
					}
					
				}
				
			}
			
			//add to array
			empties_array.emplace_back(empty);
		}
			
	}
	
}
