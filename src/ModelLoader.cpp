#include "ModelLoader.h"

#include <filesystem>
#include <algorithm>
#include <map>


ModelLoader::ModelLoader(const std::string& path){
	
	std::string error;
	std::string warning;
	
	bool load_resource = false;
	
	//check if gltf file is a binary blob or ascii and load accordingly
	if(std::filesystem::path(path).extension() == ".glb")//binary
		load_resource = tiny_gltf.LoadBinaryFromFile(&model, &error, &warning, path);//also need to add support for binary glTF loading
	else if(std::filesystem::path(path).extension() == ".gltf")//ascii
		load_resource = tiny_gltf.LoadASCIIFromFile(&model, &error, &warning, path);//also need to add support for binary glTF loading
	
	//check if successfully loaded
	if(!load_resource){
		throw std::logic_error( "Failed to load model with path: `" + path + "`. Error: " + error + ". Warning: " + warning );
	}
	
	if(model.buffers.size() > 1)
		throw std::logic_error("Only 1 buffer is currently supported.");
	if(model.buffers.empty())
		throw std::logic_error("Buffer must not be empty->");
	
	//cast the buffers to 1D arrays of common data types (float, unsigned short, etc). These arrays hold the raw vertex positions, normals, uv's, animation & skinning data etc
	float_array = (float*)model.buffers.front().data.data();
	ushort_array = (unsigned short*)model.buffers.front().data.data();
	uint_array = (unsigned int*)model.buffers.front().data.data();
	uchar_array = (unsigned char*)model.buffers.front().data.data();
	
	/////////////////
	//LOAD ALL MESHES
	/////////////////
	for(int m{}; m<model.meshes.size(); m++){
		tinygltf::Mesh& mesh = model.meshes[m];
		
		int mesh_node_idx = getMeshNodeIndex(mesh);
		tinygltf::Node& node = model.nodes[mesh_node_idx];
		
		MeshDataStruct* mesh_data_struct = new MeshDataStruct;//will store all mesh data [vertex pos/norm/uv, global mesh pos/rot etc]
		
		mesh_data_struct->name	= mesh.name;
		
		if(mesh.primitives.size() > 1)
			throw std::logic_error("Only 1 primitive per mesh is supported");
		//check to see if has sub-nodes
		if(!node.children.empty()){
			mesh_data_struct->has_childs = true;
			mesh_data_struct->childs_array = node.children;
		}
		//checks to make sure this is NOT a bone
		if( isBone(mesh_node_idx) ){
			continue;
		}
		//checks to make sure this is NOT an armature
		if( isArmature(mesh_node_idx) ){
			continue;
		}
		
		/////////////////////
		//VERTEX DATA
		/////////////////////
		mesh_data_struct->vertex_positions_array = getVertexPositions(mesh);
		
		///////////////////////
		//NORMAL DATA
		/////////////////////
		mesh_data_struct->vertex_normals_array = getVertexNormals(mesh);
		
		///////////////////////
		//UVS
		/////////////////////
		mesh_data_struct->vertex_uvs_array = getVertexUVs(mesh);
		
		///////////////////////
		//INDICES
		/////////////////////
		mesh_data_struct->vertex_indices_array = getIndices(mesh);
	
		///////////////
		//TEXTURES
		///////////////
		mesh_data_struct->texture_map = getTextureMap(mesh);
		
		///////////////////////////
		//GLOBAL POS/ROT/SCALE/MATRIX
		///////////////////////////
		{
			//find node this mesh is assigned to
			mesh_data_struct->node_index = mesh_node_idx;
			
			mesh_data_struct->translation = getTranslation(node);
			mesh_data_struct->rotation = getRotation(node);
			mesh_data_struct->scale = getScale(node);
			
			mesh_data_struct->modelMatrix = createTRSmatrix(mesh_data_struct->translation, mesh_data_struct->rotation, mesh_data_struct->scale);//if it uses a custom `matrix_transform` in the glTF, then this will get overriden so don't worry
			
			if(!node.matrix.empty()){//matrix (if specified in the file)
				mesh_data_struct->matrix_transform = getTransformMatrix(node);
				
				mesh_data_struct->has_matrix_transform = true;
				
				mesh_data_struct->modelMatrix = mesh_data_struct->matrix_transform;
			}
		}
		
		///////////////////////////
		//SKINNING [JOINTS/WEIGHTS]
		///////////////////////////
		if(!model.skins.empty()){//need better checks than this
			
			has_skin = true;//USELESS --REMOVE THIS once animation for skinning reworked [needed rn as used by animations]
			
			mesh_data_struct->has_skin = true;
			
			//joints
			mesh_data_struct->joints_array = getSkinJoints(mesh);
			
			//weights
			mesh_data_struct->weights_array = getSkinWeights(mesh);
			
			if(mesh_data_struct->joints_array.empty() || mesh_data_struct->weights_array.empty()){
				has_skin = false;
				mesh_data_struct->has_skin = false;
			}
		
			//inverse bind matrices
			mesh_data_struct->inverse_bind_matrix_array = getInverseBindMatrices(mesh);
		}
		
		///////////////////////////
		//ANIMATIONS
		///////////////////////////
		mesh_data_struct->animation_data = getMeshAnimationData(mesh);
		
		/////////////////
		//MATERIALS
		/////////////////
		mesh_data_struct->material_data = getMaterial(mesh);
		
		//add to MeshDataStruct array
		mesh_data_struct_array.emplace_back(mesh_data_struct);
	}

	///////////////////
	//LOAD ALL EMPTIES
	///////////////////
	for(int n{}; n<model.nodes.size(); n++){
		EmptyNode* empty = new EmptyNode;
		
		tinygltf::Node& node = model.nodes[n];
		
		empty->node_index = n;
		
		empty->node = node;
		
		empty->name = node.name;
		
		//checks to make sure this is NOT a mesh
		if(node.mesh != -1){
			continue;
		}
		//checks to make sure this is NOT a bone
		if( isBone(empty->node_index) ){
			continue;
		}
//		//checks to make sure this is NOT an armature
//		if( isArmature(empty->node_index) ){
//			continue;
//		}
		
		//check to see if has sub-nodes
		if(!node.children.empty()){
			empty->has_childs = true;
			empty->child_array = node.children;
		}
		
		/////////////
		//ANIMATIONS
		/////////////
		empty->animation_data = getNodeAnimationData(node);
		
		empty->is_root = isRootNode(node);
		///////////////////////////
		//GLOBAL POS/ROT/SCALE/MATRICES
		///////////////////////////
		{
			empty->translation = empty->animation_data.translation = getTranslation(node);
			empty->rotation = empty->animation_data.rotation = getRotation(node);
			empty->scale = empty->animation_data.scale = getScale(node);
			
			empty->modelMatrix = createTRSmatrix(empty->translation, empty->rotation, empty->scale);//if it uses a custom `matrix_transform` in the glTF, then this will get overriden so don't worry 
			
			if(!node.matrix.empty()){
				empty->matrix_transform = getTransformMatrix(node);
				empty->has_matrix_transform = true;
				empty->modelMatrix = empty->matrix_transform;
			}
		}
		
		//add to array
		empties_array.emplace_back(empty);
	}
	
	
	//calculate average delta animation time [based on averaged Node animations]
	{
		std::map<int, AnimationDataStruct> node_anim_map;
		for(const EmptyNode* empty : empties_array)
			node_anim_map.emplace(empty->node_index, empty->animation_data);
//		DELTA_TIME_STEP = getAveragedAnimationFps(node_anim_map);
	}
	DELTA_TIME_STEP = TIME_STEP;
	
	//////////////////////////
	//fill in Node animation gaps
	//////////////////////////
	for(EmptyNode* empty : empties_array)
		fillInAnimationGaps( empty->animation_data );
	
	//get max Node timeline
	max_node_timeline = getMaxNodeTimeline();
	//update time arrays with max array for each EmptyNode
	for(EmptyNode* empty : empties_array)
		empty->animation_data.time_array = max_node_timeline;
	
	//equalize all Node animations (fill blank arrays, match sizes)
	equalizeAndMatchNodeAnimations();

	////////////////
	//SKINNED ANIMS
	////////////////
	getSkinnedAnimation();
	
	/////////////////////////////////////////
	//fill in root-subnode relationship array
	/////////////////////////////////////////
	for(EmptyNode* empty : empties_array){
		
		//skip if bone
		if(isBone(empty->node_index))
			continue;
		
		int idx = getParentNodeIndex(empty->node);
		if(idx == -1)
			continue;
		
		std::pair<EmptyNode*, EmptyNode*> pair;
		
		//find root empty for the child
		for(EmptyNode* e_sub : empties_array){
			if(e_sub->node_index == idx){
				pair.first = e_sub;
				break;
			}
		}
		
		//child empty
		pair.second = empty;
		
		//add to array
		root_and_child_array.emplace_back(pair);
	}
	
	///////////////////////////////////////////
	//FOR EDGE CASES -- fill in all root nodes
	///////////////////////////////////////////
	for(EmptyNode* empty : empties_array){
		//skip if bone
		if(isBone(empty->node_index))
			continue;
		
		int idx = getParentNodeIndex(empty->node);
		if(idx == -1)//MUST be == -1 for it to be a root node
			root_array.emplace_back(empty);
	}
	
	
	//check to see if skin and model anims are synced
	for(auto b : bone_animation_channel_map){
		if(!b.second.empty() && !max_node_timeline.empty())
			if(b.second.front().time_array.size() != max_node_timeline.size())
				PRINT_WARN("Warning: detected possible desync between skinned and model animations.");
	}
	
}

ModelLoader::~ModelLoader() {
	
	//delete all GL texture's once finished
	for(MeshDataStruct* mesh_data : mesh_data_struct_array){
		for(const auto& tex_data : mesh_data->texture_map){
			glDeleteTextures(1, &tex_data.second.tex_id);
		}
	}

	//delete all MeshDataStruct objects
	for(MeshDataStruct* mesh_data : mesh_data_struct_array){
		delete mesh_data;
		mesh_data = nullptr;
	}
	
	//delete all EmptyNode objects
	for(EmptyNode* empty : empties_array){
		delete empty;
		empty = nullptr;
	}
	
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
	
	if(vert_pos_accessor.byteOffset != 0)
		byteOffset = vert_pos_accessor.byteOffset + byteOffset;
	
	int offset = byteOffset/sizeof(float);
//	std::cout << mesh.name << " byteoffset ACCESSOR  " << vert_pos_accessor.byteOffset << std::endl;
//	std::cout << mesh.name << " byteoffset  BUFVIEW " << model.bufferViews[vert_pos_accessor.bufferView].byteOffset << std::endl;
	
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
	
	if(vert_norm_accessor.byteOffset != 0)
		byteOffset = vert_norm_accessor.byteOffset + byteOffset;
	
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
	
	if(vert_uv_accessor.byteOffset != 0)
		byteOffset = vert_uv_accessor.byteOffset + byteOffset;
	
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
	
	if(vert_index_accessor.byteOffset != 0)
		byteOffset = vert_index_accessor.byteOffset + byteOffset;
	
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

std::map<TextureType, TextureDataStruct> ModelLoader::getTextureMap(const tinygltf::Mesh& mesh){
	
	std::map<TextureType, TextureDataStruct> texture_data_array;
	
	if(model.textures.empty() || model.materials.empty()){
		return texture_data_array; 
	}
	
	tinygltf::Primitive prim = mesh.primitives.front();
		
	int mat_idx = prim.material;
	
	if(mat_idx == -1)
		return texture_data_array;
	
	tinygltf::Material mat = model.materials[mat_idx];
	
	int diff_tex_idx = mat.pbrMetallicRoughness.baseColorTexture.index;
	int normal_tex_idx = mat.normalTexture.index;
	int metallic_tex_idx = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
	
	//////////////////
	//diffuse texture
	//////////////////
	if(diff_tex_idx != -1){
		tinygltf::Texture diffuse_textu = model.textures[diff_tex_idx];
		int diff_img_idx = diffuse_textu.source; 
		
		tinygltf::Image diffuse_image = model.images[diff_img_idx];
		
		////////////////////
		//generate GL texture
		////////////////////
		glGenTextures(1, &diffuse_texture);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//from ex
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //far away texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //close up texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diffuse_image.width, diffuse_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuse_image.image.data());
		
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, 0);//unbind once we've finished [by binding to 0]
		
		__GL_ERROR_THROW__("Texture creation failed"); //check for GL any errors
		
		//adds to array
		texture_data_array.emplace( TextureType::DIFFUSE, TextureDataStruct{TextureType::DIFFUSE, diffuse_texture} );
	}
	//////////////////
	//normal texture
	//////////////////
	if(normal_tex_idx != -1){
		tinygltf::Texture normal_textu = model.textures[normal_tex_idx];
		int norm_img_idx = normal_textu.source; 
		
		tinygltf::Image normal_image = model.images[norm_img_idx];
		
		////////////////////
		//generate GL texture
		////////////////////
		glGenTextures(1, &normal_texture);
		glBindTexture(GL_TEXTURE_2D, normal_texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//from ex
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //far away texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //close up texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, normal_image.width, normal_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, normal_image.image.data());
		
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, 0);//unbind once we've finished [by binding to 0]
		
		__GL_ERROR_THROW__("Texture creation failed"); //check for GL any errors
		
		//adds to array
		texture_data_array.emplace( TextureType::NORMAL, TextureDataStruct{TextureType::NORMAL, normal_texture} );
	}
	//////////////////
	//metallic texture
	//////////////////
	if(metallic_tex_idx != -1){
		tinygltf::Texture metal_textu = model.textures[metallic_tex_idx];
		int metal_img_idx = metal_textu.source; 
		
		tinygltf::Image metal_image = model.images[metal_img_idx];
		
		////////////////////
		//generate GL texture
		////////////////////
		glGenTextures(1, &metal_texture);
		glBindTexture(GL_TEXTURE_2D, metal_texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//from ex
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //far away texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //close up texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, metal_image.width, metal_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, metal_image.image.data());
		
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, 0);//unbind once we've finished [by binding to 0]
		
		__GL_ERROR_THROW__("Texture creation failed"); //check for GL any errors
		
		//adds to array
		texture_data_array.emplace( TextureType::METAL, TextureDataStruct{TextureType::METAL, metal_texture} );
	}
	
	return texture_data_array;

}

MaterialDataStruct ModelLoader::getMaterial(const tinygltf::Mesh& mesh){
	
	MaterialDataStruct material_data;
	
	tinygltf::Primitive prim = mesh.primitives.front();
	
	int mat_idx = prim.material;
	
	if(model.materials.empty() || mat_idx == -1)
		return material_data;
	
	tinygltf::Material mat = model.materials[mat_idx];

	if(!mat.pbrMetallicRoughness.baseColorFactor.empty())
		material_data.base_color = glm::vec4( mat.pbrMetallicRoughness.baseColorFactor[0], mat.pbrMetallicRoughness.baseColorFactor[1], mat.pbrMetallicRoughness.baseColorFactor[2], mat.pbrMetallicRoughness.baseColorFactor[3] );
	
	material_data.has_material = true;
	
	material_data.name = mat.name;
	
	material_data.metalness = mat.pbrMetallicRoughness.metallicFactor;
	
	material_data.roughness = mat.pbrMetallicRoughness.roughnessFactor;
	
	return material_data;
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
//				return m;
				return n;//RETURN n
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
				
				break;
			}
		}
	}
	
	//if no animations for the current mesh, break out
	if(!animations_found){
		return animation_data;
	}
	
	for(int c{}; c<animation.channels.size(); c++){
		
		animation_data.has_animation = true;
		
		/////////////////////
		//fetch times
		/////////////////////
		tinygltf::AnimationSampler& time_sampler = animation.samplers[0];
		std::vector<float> times = getTimelineArray(time_sampler);
		animation_data.time_array = times;
		

		tinygltf::AnimationChannel& channel = animation.channels[c];
		tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
		
		int node_idx = channel.target_node;
		
		//MAKE SURE ITS NOT BONE
		if(isBone(node_idx)){
			continue;
		}
		
		for(std::size_t n{}; n<model.nodes.size(); n++){
			std::vector<int> childs_vec = model.nodes[n].children;
			
//			if(childs_vec.size()>1)
//				PRINT_WARN("IMPROVE SYSTEM FOR MORE THAN 1 CHILD");
			
			if( std::find(childs_vec.begin(), childs_vec.end(), node_idx) != childs_vec.end() ){
				animation_data.has_root = true;
				animation_data.root_idx = n;
				break;
			}
			
		}
		
		
		int output_idx = sampler.output;
		
		std::string target_path = channel.target_path;
		tinygltf::Accessor& accessor = model.accessors[output_idx];
		int frame_count = model.accessors[output_idx].count;
		int byteOffset = model.bufferViews[accessor.bufferView].byteOffset;
		
		if(accessor.byteOffset != 0)
			byteOffset = accessor.byteOffset + byteOffset;
		
		int offset = byteOffset/getSizeOfComponentType(accessor.componentType);
		for(int i{}; i<frame_count; i++){
			
			//translations
			if(target_path == "translation"){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				animation_data.translation_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "translation data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				
				//fetch timeline for translation channel
				animation_data.trans_time_array = getTimelineArray(sampler);
			}
			
			//rotations
			if(target_path == "rotation"){
				float x = float_array[(i*4) + 0 + offset];
				float y = float_array[(i*4) + 1 + offset];
				float z = float_array[(i*4) + 2 + offset];
				float w = float_array[(i*4) + 3 + offset];
				animation_data.rotation_anim_array.emplace_back( glm::quat(w, x, y, z) );
//					std::cout << "rotation data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
				
				//fetch timeline for rot channel
				animation_data.rot_time_array = getTimelineArray(sampler);
			}
			
			//scale
			if(target_path == "scale"){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				
				//fetch timeline for scale channel
				animation_data.scale_time_array = getTimelineArray(sampler);
			}
			
		}
		
	}
	
	//adds check to ensure all arrays are equal [will be fixed soon]
	if( animation_data.translation_anim_array.size() != animation_data.rotation_anim_array.size() || animation_data.translation_anim_array.size() != animation_data.scale_anim_array.size() || animation_data.rotation_anim_array.size() != animation_data.scale_anim_array.size() ){
		equalizeTRSanimationArrays(animation_data);
//		PRINT_WARN("Translation, scale and rotation animation durations must be equal.");
		throw std::logic_error("Mesh translation, scale and rotation animation durations must be equal [reminder to implement animation gap filling for meshes as well].");
	}
	
	return animation_data;
}

AnimationDataStruct ModelLoader::getNodeAnimationData(const tinygltf::Node& node){
	
	AnimationDataStruct animation_data;
	
	///////////////////////////
	//ANIMATIONS
	///////////////////////////
	if(model.animations.empty()){
		return animation_data;
	}
	
	tinygltf::Animation animation;
	
	//index in the node array
	int node_idx = std::distance(model.nodes.begin(), std::find(model.nodes.begin(), model.nodes.end(), node));
	
	//MAKE SURE IT'S NOT A BONE
	if(isBone(node_idx)){
		return animation_data;
	}
	
	//find the tinygltf::Animation object for this empty/node
	bool animations_found = false;
	for(const tinygltf::Animation& anim : model.animations){
		for(const tinygltf::AnimationChannel& chan : anim.channels){
			if(chan.target_node == node_idx){
				animation = anim;
				animations_found = true;
				break;
			}
		}
	}
	
	//if no animations for the current empty/node, break out
	if(!animations_found){
		return animation_data;
	}
	
	animation_data.has_animation = true;
	
	for(int c{}; c<animation.channels.size(); c++){
		/////////////////////
		//fetch times
		/////////////////////
		tinygltf::AnimationSampler& time_sampler = animation.samplers[0];
//		std::vector<float> times = getTimelineArray(time_sampler);
		
		tinygltf::AnimationChannel& channel = animation.channels[c];
		tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
		
		int input_idx = channel.target_node;
		int output_idx = sampler.output;
		
		//detect different animations within the same channel
		if(input_idx != node_idx){
			continue;
		}
		animation_data.has_animation = true;
		
//		animation_data.time_array = times;
		
		animation_data.name = animation.name;
		
		//MAKES SURE THIS ISN'T A BONE [just an empty]
		if(isBone(input_idx)){
			continue;
		}
		
		std::string target_path = channel.target_path;
		tinygltf::Accessor& accessor = model.accessors[output_idx];
		int frame_count = model.accessors[output_idx].count;
		int byteOffset = model.bufferViews[accessor.bufferView].byteOffset;
		
		if(accessor.byteOffset != 0)
			byteOffset = accessor.byteOffset + byteOffset;
		
		int offset = byteOffset/getSizeOfComponentType(accessor.componentType);
		
		
		for(int i{}; i<frame_count; i++){
			
			//translations
			if(target_path == "translation"){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				animation_data.translation_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "translation data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				animation_data.trans_time_array = getTimelineArray(sampler);
				
				node_timelines_map.emplace(animation_data.trans_time_array.size(), animation_data.trans_time_array);
			}
			
			//rotations
			if(target_path == "rotation"){
				float x = float_array[(i*4) + 0 + offset];
				float y = float_array[(i*4) + 1 + offset];
				float z = float_array[(i*4) + 2 + offset];
				float w = float_array[(i*4) + 3 + offset];
				animation_data.rotation_anim_array.emplace_back( glm::quat(w, x, y, z) );
//					std::cout << "rotation data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
				
				animation_data.rot_time_array = getTimelineArray(sampler);
				
				node_timelines_map.emplace(animation_data.rot_time_array.size(), animation_data.rot_time_array);
			}
			
			//scale
			if(target_path == "scale"){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				
				animation_data.scale_time_array = getTimelineArray(sampler);
				
				node_timelines_map.emplace(animation_data.scale_time_array.size(), animation_data.scale_time_array);
			}
			
		}
		
	}
	
	return animation_data;
}

std::vector<float> ModelLoader::getMaxNodeTimeline(){
	std::map<int, std::vector<float>> sorted_timelines;
	
	for(const EmptyNode* empty : empties_array){
		const AnimationDataStruct& animation_data = empty->animation_data;
		sorted_timelines.emplace(animation_data.trans_time_array.size(), animation_data.trans_time_array);
		sorted_timelines.emplace(animation_data.rot_time_array.size(), animation_data.rot_time_array);
		sorted_timelines.emplace(animation_data.scale_time_array.size(), animation_data.scale_time_array);
	}
	
	std::vector<float> max_timeline;
	
	//return empty array if no anims
	if(sorted_timelines.empty())
		return max_timeline;
	
	max_timeline = sorted_timelines.rbegin()->second;
	
	//return empty array if no anims
	if(max_timeline.empty())
		return max_timeline;
	
	//check for any time gaps and fill in
		std::map<int, int> gap_idx_trans_array;
		int summed_gap_size {};
		for(int y{}; y<max_timeline.size() - 1; y++){
			float curr_delta = max_timeline[y+1]-max_timeline[y];
			float delta_time = DELTA_TIME_STEP;
			int gap_steps = std::round(curr_delta/delta_time);
//			std::cout << y << " " << "ti " << animation_data.trans_time_array[y] << std::endl;
			if(curr_delta >= delta_time*1.2f){
//					std::cout << "old index: " << y << ", time " << curr_delta << ", gaps " << gap_steps << ", NEW IDX " << y+summed_gap_size << std::endl;  
				gap_idx_trans_array.emplace(y + summed_gap_size, gap_steps - 1);
				
				summed_gap_size += gap_steps - 1; 
			}
		}
		//insert missing times
		for(const auto& itr : gap_idx_trans_array){
			float delta = max_timeline[itr.first] + DELTA_TIME_STEP;
			for(int i{}; i<itr.second; i++){
				max_timeline.insert(max_timeline.begin() + itr.first + 1 + i, delta);
				delta += DELTA_TIME_STEP;
			}
		}
	
//	std::cout << "max " << max_timeline.size() << std::endl;
//	for(int t{}; t<max_timeline.size(); t++)
//		std::cout << "idx " << t << "   time " << max_timeline[t] << std::endl;
	
	return max_timeline;
}

std::vector<float> ModelLoader::getMaxSkinnedTimeline(const std::map<int, AnimationDataStruct>& bone_anim_map){
	std::map<int, std::vector<float>> sorted_timelines;
	
	for(const auto& itr : bone_anim_map){
		const AnimationDataStruct& animation_data = itr.second;
		sorted_timelines.emplace(animation_data.trans_time_array.size(), animation_data.trans_time_array);
		sorted_timelines.emplace(animation_data.rot_time_array.size(), animation_data.rot_time_array);
		sorted_timelines.emplace(animation_data.scale_time_array.size(), animation_data.scale_time_array);
	}
	
	std::vector<float> max_timeline;
	
	if(sorted_timelines.empty())
		return max_timeline;
	
	max_timeline = sorted_timelines.rbegin()->second;
	
	//check for any time gaps and fill in
	std::map<int, int> gap_idx_trans_array;
	int summed_gap_size {};
	for(int y{}; y<max_timeline.size() - 1; y++){
		float curr_delta = max_timeline[y+1]-max_timeline[y];
		float delta_time = DELTA_TIME_STEP;
		int gap_steps = std::round(curr_delta/delta_time);
//			std::cout << y << " " << "ti " << animation_data.trans_time_array[y] << std::endl;
		if(curr_delta >= delta_time*1.2f){
//					std::cout << "old index: " << y << ", time " << curr_delta << ", gaps " << gap_steps << ", NEW IDX " << y+summed_gap_size << std::endl;  
			gap_idx_trans_array.emplace(y + summed_gap_size, gap_steps - 1);
			
			summed_gap_size += gap_steps - 1; 
		}
	}
	//insert missing times
	for(const auto& itr : gap_idx_trans_array){
		float delta = max_timeline[itr.first] + DELTA_TIME_STEP;
		for(int i{}; i<itr.second; i++){
			max_timeline.insert(max_timeline.begin() + itr.first + 1 + i, delta);
			delta += DELTA_TIME_STEP;
		}
	}
	
//	std::cout << "max " << max_timeline.size() << std::endl;
//	for(int t{}; t<max_timeline.size(); t++)
//		std::cout << "idx " << t << "   time " << max_timeline[t] << std::endl;
	
	return max_timeline;
}

float ModelLoader::getMaxSkinnedDuration(const std::map<int, AnimationDataStruct>& bone_anim_map){
	std::map<int, std::vector<float>> sorted_timelines;
	
	for(const auto& itr : bone_anim_map){
		const AnimationDataStruct& animation_data = itr.second;
		sorted_timelines.emplace(animation_data.trans_time_array.size(), animation_data.trans_time_array);
		sorted_timelines.emplace(animation_data.rot_time_array.size(), animation_data.rot_time_array);
		sorted_timelines.emplace(animation_data.scale_time_array.size(), animation_data.scale_time_array);
	}
	
	if(sorted_timelines.empty()){
		PRINT_COLOR("Returning 0.f", 255, 0, 0);
		return 0.f;
	}
	
	std::vector<float> sorted;
	
	for(const auto& itr : sorted_timelines){
		for(float t : itr.second)
			sorted.emplace_back(t);
	}
	
	std::sort(sorted.begin(), sorted.end());
	
	float maxi = sorted.back();
	
	return maxi;
}

void ModelLoader::equalizeAndMatchNodeAnimations(){
	////////////////////////////////////
	//makes sure all arrays are not empty [fills them in up to max time size]
	////////////////////////////////////
	for(EmptyNode* empty : empties_array){
		AnimationDataStruct& animation_data = empty->animation_data;
		//pos
		if(animation_data.translation_anim_array.empty())
			for(float t : max_node_timeline)
				animation_data.translation_anim_array.emplace_back(animation_data.translation);
		//rots
		if(animation_data.rotation_anim_array.empty())
			for(float t : max_node_timeline)
				animation_data.rotation_anim_array.emplace_back(animation_data.rotation);
		//scales
		if(animation_data.scale_anim_array.empty())
			for(float t : max_node_timeline)
				animation_data.scale_anim_array.emplace_back(animation_data.scale);
	}
	
	////////////////////////////////////
	//makes sure all aniamtion arrays are of equal length (pos/rot/scale)
	////////////////////////////////////
	for(EmptyNode* empty : empties_array){
		AnimationDataStruct& animation_data = empty->animation_data;
		int max_size = max_node_timeline.size();
		
		//equalize translation array
		if(animation_data.translation_anim_array.size() != max_size){
			//if few frames short, calc difference and apply
			int diff = max_size - animation_data.translation_anim_array.size();
			glm::vec3 last_pos = animation_data.translation_anim_array.back();
			for(int i{}; i<diff; i++)
				animation_data.translation_anim_array.emplace_back( last_pos );
		}
		
		//equalize rot array
		if(animation_data.rotation_anim_array.size() != max_size){
			//if few frames short, calc difference and apply
			int diff = max_size - animation_data.rotation_anim_array.size();
			glm::quat last_rot = animation_data.rotation_anim_array.back();
			for(int i{}; i<diff; i++)
				animation_data.rotation_anim_array.emplace_back( last_rot );
		}
		
		//equalize scale array
		if(animation_data.scale_anim_array.size() != max_size){
			//if few frames short, calc difference and apply
			int diff = max_size - animation_data.scale_anim_array.size();
			glm::vec3 last_scale = animation_data.scale_anim_array.back();
			for(int i{}; i<diff; i++)
				animation_data.scale_anim_array.emplace_back( last_scale );
		}
		
	}
}

void ModelLoader::getSkinnedAnimation(){
	
	if(model.skins.empty())
		return;
	
	if(model.skins.size() > 1)
		PRINT_WARN("Only 1 skinned animation per ModelLoader instance is supported as of now.");
	
	tinygltf::Skin& skin = model.skins.front();
	
	std::vector<int> skin_node_indices = skin.joints;
	
	///////////////////////////
	//ANIMATIONS
	///////////////////////////
	if(model.animations.empty()){
		return;
	}
	
	//more than 1 animation?
	std::map<std::string, tinygltf::Animation> animation_gltf_map;
	
	//find animation channels for this skin
	for(const tinygltf::Animation& a : model.animations){
		for(const tinygltf::AnimationChannel& c : a.channels){
			if(std::find(skin_node_indices.begin(), skin_node_indices.end(), c.target_node) != skin_node_indices.end()){
				animation_gltf_map.emplace(a.name, a);
			}
		}
	}
		
	for(const auto& anim_map_itr : animation_gltf_map){
		
		tinygltf::Animation anim = anim_map_itr.second;
		
		//stores timeline array for EACH bone
		std::map<int, std::vector<float>> timeline_map;
		
		//stores anim data for EACH bone
		std::map<int, AnimationDataStruct> bone_anim_map;
		
		/////////////////////////////////////////
		//fetch animation timeline for each bone
		/////////////////////////////////////////
		for(int c{}; c<anim.channels.size(); c++){
			
			tinygltf::AnimationChannel& channel = anim.channels[c];
			tinygltf::AnimationSampler& sampler = anim.samplers[channel.sampler];
			
			//the node it belongs to
			int node_idx = channel.target_node;
			//checks to make sure this is NOT an armature
			if( isArmature(node_idx) ){
				continue;
			}
			//MAKE SURE ITS A BONE
			if(!isBone(node_idx)){
				continue;
			}
			
			/////////////////////
			//fetch times
			/////////////////////
			tinygltf::AnimationSampler& time_sampler = anim.samplers[0];
	//		std::vector<float> times = getTimelineArray(time_sampler);
			
			int output_idx = sampler.output;
			
			std::string target_path = channel.target_path;
			tinygltf::Accessor& accessor = model.accessors[output_idx];
			int frame_count = model.accessors[output_idx].count;
			int byteOffset = model.bufferViews[accessor.bufferView].byteOffset;
			
			if(accessor.byteOffset != 0)
				byteOffset = accessor.byteOffset + byteOffset;
			
			int offset = byteOffset/getSizeOfComponentType(accessor.componentType);
			
			for(int i{}; i<frame_count; i++){
				//translations
				if(target_path == "translation"){
					//add to map
					timeline_map.emplace(getTimelineArray(sampler).size(), getTimelineArray(sampler));
				}
				
				//rotations
				if(target_path == "rotation"){
					//add to map
					timeline_map.emplace(getTimelineArray(sampler).size(), getTimelineArray(sampler));
				}
				
				//scale
				if(target_path == "scale"){
					//add to map
					timeline_map.emplace(getTimelineArray(sampler).size(), getTimelineArray(sampler));
				}
				
			}
			
			
		}
		
		//add anim for each bone [will be filled in later]
		for(int joint_idx : skin.joints){
			AnimationDataStruct animation_data;
			animation_data.node_index = joint_idx;
			animation_data.node = model.nodes[joint_idx];
	//		animation_data.time_array = max_time_array;
			bone_anim_map.emplace(joint_idx, animation_data);
		}
			
		/////////////////////////////////////////
		//fetch animation trans/rot/scale for each bone
		/////////////////////////////////////////
		static std::size_t idx {};//used to keep track of individual bones
		for(int c{}; c<anim.channels.size(); c++){
			
			tinygltf::AnimationChannel& channel = anim.channels[c];
			tinygltf::AnimationSampler& sampler = anim.samplers[channel.sampler];
			
			//the node it belongs to
			int node_idx = channel.target_node;
			//checks to make sure this is NOT an armature
			if( isArmature(node_idx) ){
				continue;
			}
			//MAKE SURE ITS A BONE
			if(!isBone(node_idx)){
				continue;
			}
			
			///////////////////////
			//fetch translations/rots/scale
			///////////////////////
			AnimationDataStruct& animation_data = bone_anim_map[node_idx];
			
			animation_data.has_animation = true;
			animation_data.node = model.nodes[node_idx];
			///////////////
			///////////////
			///////////////
			//FIND ROOT BONE
			for(std::size_t n{}; n<model.nodes.size(); n++){
				std::vector<int> childs_vec = model.nodes[n].children;
				if( std::find(childs_vec.begin(), childs_vec.end(), node_idx) != childs_vec.end() ){
					animation_data.has_root = true;
					animation_data.root_idx = n;
					break;
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
			
			if(accessor.byteOffset != 0)
				byteOffset = accessor.byteOffset + byteOffset;
			
			int offset = byteOffset/getSizeOfComponentType(accessor.componentType);
			
			for(int i{}; i<frame_count; i++){
				//translations
				if(target_path == "translation"){
					float x = float_array[(i*3) + 0 + offset];
					float y = float_array[(i*3) + 1 + offset];
					float z = float_array[(i*3) + 2 + offset];
					animation_data.translation_anim_array.emplace_back( glm::vec3(x, y, z) );
		//					std::cout << "translation data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
					
					animation_data.trans_time_array = getTimelineArray(sampler);
					
	//				animation_data.time_array = max_time_array;
				}
				
				//rotations
				if(target_path == "rotation"){
					float x = float_array[(i*4) + 0 + offset];
					float y = float_array[(i*4) + 1 + offset];
					float z = float_array[(i*4) + 2 + offset];
					float w = float_array[(i*4) + 3 + offset];
					animation_data.rotation_anim_array.emplace_back( glm::quat(w, x, y, z) );
		//					std::cout << "rotation data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
					
					animation_data.rot_time_array = getTimelineArray(sampler);
					
	//				animation_data.time_array = max_time_array;
				}
				
				//scale
				if(target_path == "scale"){
					float x = float_array[(i*3) + 0 + offset];
					float y = float_array[(i*3) + 1 + offset];
					float z = float_array[(i*3) + 2 + offset];
					animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
		//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
					
					animation_data.scale_time_array = getTimelineArray(sampler);
					
	//				animation_data.time_array = max_time_array;
				}
			
		}
		
	}
		
		
		//////////////////////////////////////////
		//////////////////////////////////////////
		//////////////////////////////////////////
		//////////////////////////////////////////
		// NEW REMOVE FRAMES THING
		// NEW REMOVE FRAMES THING
		// NEW REMOVE FRAMES THING
		// NEW REMOVE FRAMES THING
		/*
		*/
		
		std::vector<float> time_array;
		//	int array_size = animation_data.trans_time_array.back()/0.04166666666;
		float tt {};
		float max_time = getMaxSkinnedDuration(bone_anim_map);
		while(tt<max_time){
			time_array.emplace_back(tt);
			tt+=TIME_STEP;
		}
		
		std::vector<std::string> time_array_str;
		for(float f : time_array){
			time_array_str.emplace_back( std::to_string(f) );
		}
		
		//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&7
		//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&7
		for(auto& v : bone_anim_map){
			
			//		if(v.second.node.name != "Frame_01")
			//			continue;
			
			AnimationDataStruct& animation_data = v.second;
			
			if(anim_map_itr.second.name == "v_shotgun.qc_skeleton|after_reload")
			{
				std::cout << "BEFORE trans " << animation_data.trans_time_array.size() << ", rot " << animation_data.rot_time_array.size() << ", scale " << animation_data.scale_time_array.size() << std::endl;
			}

			/*
			
			/////////////////////
			/////////////////////
			//REMOVE FRAMES
			/////////////////////
			//TRANS
			{
				int removed_cnt = 0;
				std::vector<int> gap_idx_map;
				for(int i{}; i<animation_data.trans_time_array.size(); i++){
					if( std::find(time_array_str.begin(), time_array_str.end(), std::to_string(animation_data.trans_time_array[i])) == time_array_str.end() ){
						//					PRINT_WARN(std::to_string(animation_data.trans_time_array[i]) + " bad value");
						gap_idx_map.emplace_back(i - removed_cnt);
						//					std::cout << "remove " << i - removed_cnt << std::endl;
						removed_cnt += 1;
					}
				}
				for(int i : gap_idx_map){
					animation_data.trans_time_array.erase(animation_data.trans_time_array.begin() + i);
					animation_data.translation_anim_array.erase(animation_data.translation_anim_array.begin() + i);
				}
			}
			//ROT
			{
				int removed_cnt = 0;
				std::vector<int> gap_idx_map;
				for(int i{}; i<animation_data.rot_time_array.size(); i++){
					if( std::find(time_array_str.begin(), time_array_str.end(), std::to_string(animation_data.rot_time_array[i])) == time_array_str.end() ){
						//					PRINT_WARN(std::to_string(animation_data.rot_time_array[i]) + " bad value");
						gap_idx_map.emplace_back(i - removed_cnt);
						removed_cnt += 1;
					}
				}
				for(int i : gap_idx_map){
					animation_data.rot_time_array.erase(animation_data.rot_time_array.begin() + i);
					animation_data.rotation_anim_array.erase(animation_data.rotation_anim_array.begin() + i);
				}
			}
			//SCALE
			{
				int removed_cnt = 0;
				std::vector<int> gap_idx_map;
				for(int i{}; i<animation_data.scale_time_array.size(); i++){
					if( std::find(time_array_str.begin(), time_array_str.end(), std::to_string(animation_data.scale_time_array[i])) == time_array_str.end() ){
						//					PRINT_WARN(std::to_string(animation_data.scale_time_array[i]) + " bad value");
						gap_idx_map.emplace_back(i - removed_cnt);
						removed_cnt += 1;
					}
				}
				for(int i : gap_idx_map){
					animation_data.scale_time_array.erase(animation_data.scale_time_array.begin() + i);
					animation_data.scale_anim_array.erase(animation_data.scale_anim_array.begin() + i);
				}
			}
			*/
			fillInAnimationGaps(animation_data);
			
			
			/*
			*/
			/////////////////////
			/////////////////////
			//REMOVE FRAMES
			/////////////////////
			//TRANS
			{
				int removed_cnt = 0;
				std::vector<int> gap_idx_map;
				for(int i{}; i<animation_data.trans_time_array.size(); i++){
					if( std::abs( std::remainderf(animation_data.trans_time_array[i], TIME_STEP) ) > 0.00001f ){
//											PRINT_WARN(std::to_string(animation_data.trans_time_array[i]) + " bad value");
						gap_idx_map.emplace_back(i - removed_cnt);
						//					std::cout << "remove " << i - removed_cnt << std::endl;
						removed_cnt += 1;
					}
				}
				for(int i : gap_idx_map){
					animation_data.trans_time_array.erase(animation_data.trans_time_array.begin() + i);
					animation_data.translation_anim_array.erase(animation_data.translation_anim_array.begin() + i);
				}
			}
			//ROT
			{
				int removed_cnt = 0;
				std::vector<int> gap_idx_map;
				for(int i{}; i<animation_data.rot_time_array.size(); i++){
					if( std::abs( std::remainderf(animation_data.rot_time_array[i], TIME_STEP) ) > 0.00001f ){
						//					PRINT_WARN(std::to_string(animation_data.rot_time_array[i]) + " bad value");
						gap_idx_map.emplace_back(i - removed_cnt);
						removed_cnt += 1;
					}
				}
				for(int i : gap_idx_map){
					animation_data.rot_time_array.erase(animation_data.rot_time_array.begin() + i);
					animation_data.rotation_anim_array.erase(animation_data.rotation_anim_array.begin() + i);
				}
			}
			//SCALE
			{
				int removed_cnt = 0;
				std::vector<int> gap_idx_map;
				for(int i{}; i<animation_data.scale_time_array.size(); i++){
					if( std::abs( std::remainderf(animation_data.scale_time_array[i], TIME_STEP) ) > 0.00001f ){
						//					PRINT_WARN(std::to_string(animation_data.scale_time_array[i]) + " bad value");
						gap_idx_map.emplace_back(i - removed_cnt);
						removed_cnt += 1;
					}
				}
				for(int i : gap_idx_map){
					animation_data.scale_time_array.erase(animation_data.scale_time_array.begin() + i);
					animation_data.scale_anim_array.erase(animation_data.scale_anim_array.begin() + i);
				}
			}
			
			
			
			if(anim_map_itr.second.name == "v_shotgun.qc_skeleton|after_reload")
			{
				std::cout << "AFTER trans " << animation_data.trans_time_array.size() << ", rot " << animation_data.rot_time_array.size() << ", scale " << animation_data.scale_time_array.size() << std::endl;
			}
			
			
			animation_data.trans_time_array = time_array;
			animation_data.rot_time_array = time_array;
			animation_data.scale_time_array = time_array;
			
		}	
		
		
		
		
		
		//get static translations/rots/scales for each bone
		for(auto& itr : bone_anim_map){
			AnimationDataStruct& animation_data = itr.second;
			const tinygltf::Node& bone_node = model.nodes[ animation_data.node_index ];
			
			//trans
			if(!bone_node.translation.empty()){
				animation_data.translation = glm::vec3( bone_node.translation[0], bone_node.translation[1], bone_node.translation[2] ) ;
			}
			//rot
			if(!bone_node.rotation.empty()){
				animation_data.rotation = glm::quat( bone_node.rotation[3], bone_node.rotation[0], bone_node.rotation[1], bone_node.rotation[2] ) ;
			}
			//scale
			if(!bone_node.scale.empty()){
				animation_data.scale = glm::vec3( 1.f ) ;
			}
			
		}
		
		
		//calculate delta time [if not already done so by model anims, when they dont exist]
		if(DELTA_TIME_STEP == 0.f)//reminder for me to implement delta time calc fallback for bones, in case mdoels anims dont exist
		{
//			DELTA_TIME_STEP = getAveragedAnimationFps(bone_anim_map);
		}
		DELTA_TIME_STEP = TIME_STEP;
	
		
		
		/*
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		////////////////////////////
		//FILLS IN ANIMATION GAPS
		/////////////////////////////
		for(auto& itr : bone_anim_map){
			AnimationDataStruct& animation_data = itr.second;
			fillInAnimationGaps(animation_data);
		}
		*/
		
		
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		//COMMENTED OUT FOR NOW -- RE-ADD IT LATER
		//get max timeline array
	//	std::vector<float> max_timeline = getMaxSkinnedTimeline(bone_anim_map);
		std::vector<float> max_timeline = time_array;
	
		//update time_array with max timeline for each bone
		for(auto& itr : bone_anim_map){
			AnimationDataStruct& animation_data = itr.second;
			animation_data.time_array = max_timeline;
		}
		
		//mark all non-anim bones as being animated [not-needed?]
		for(auto& v : bone_anim_map){
			if(!v.second.has_animation){
				AnimationDataStruct& animation_data = v.second;
				animation_data.has_animation = true;
	//			PRINT_COLOR("bone with no anim data -> " + model.nodes[anim.node_index].name,255,0,0);
			}
		}
		
		//find root bones 
		for(auto& v : bone_anim_map){
			AnimationDataStruct& animation_data = v.second;
			for(std::size_t n{}; n<model.nodes.size(); n++){
				std::vector<int> childs_vec = model.nodes[n].children;
				if( std::find(childs_vec.begin(), childs_vec.end(), animation_data.node_index) != childs_vec.end() ){
					animation_data.has_root = true;
					animation_data.root_idx = n;
					break;
				}
				
			}
		}
		
		//equalize empty arrays
		for(auto& v : bone_anim_map){
			AnimationDataStruct& animation_data = v.second;
			
			//if size == 0
			//pos
			if(animation_data.translation_anim_array.empty())
				for(float t : max_timeline)
					animation_data.translation_anim_array.emplace_back(animation_data.translation);
			//if size == 1
			if(animation_data.translation_anim_array.size() == 1)
				for(int y{}; y<max_timeline.size() - 1; y++)
					animation_data.translation_anim_array.emplace_back(animation_data.translation_anim_array.front());
			
			//if size == 0
			//rots
			if(animation_data.rotation_anim_array.empty())
				for(float t : max_timeline)
					animation_data.rotation_anim_array.emplace_back(animation_data.rotation);
			//if size == 1
			if(animation_data.rotation_anim_array.size() == 1)
				for(int y{}; y<max_timeline.size() - 1; y++)
					animation_data.rotation_anim_array.emplace_back(animation_data.rotation_anim_array.front());
			
			//if size == 0
			//scales
			if(animation_data.scale_anim_array.empty())
				for(float t : max_timeline)
					animation_data.scale_anim_array.emplace_back(animation_data.scale);
			//if size == 1
			if(animation_data.scale_anim_array.size() == 1)
				for(int y{}; y<max_timeline.size() - 1; y++)
					animation_data.scale_anim_array.emplace_back(animation_data.scale_anim_array.front());
	
		}
		
		//match up array sizes to be as large as max timeline
		for(auto& v : bone_anim_map){
			AnimationDataStruct& animation_data = v.second;
			
			int max_size = max_timeline.size();
			
			//equalize translation array
			if(animation_data.translation_anim_array.size() != max_size){
				//if few frames short, calc difference and apply
				int diff = max_size - animation_data.translation_anim_array.size();
				glm::vec3 last_pos = animation_data.translation_anim_array.back();
				for(int i{}; i<diff; i++)
					animation_data.translation_anim_array.emplace_back( last_pos );
			}
			
			//equalize rot array
			if(animation_data.rotation_anim_array.size() != max_size){
				//if few frames short, calc difference and apply
				int diff = max_size - animation_data.rotation_anim_array.size();
				glm::quat last_rot = animation_data.rotation_anim_array.back();
				for(int i{}; i<diff; i++)
					animation_data.rotation_anim_array.emplace_back( last_rot );
			}
			
			//equalize scale array
			if(animation_data.scale_anim_array.size() != max_size){
				//if few frames short, calc difference and apply
				int diff = max_size - animation_data.scale_anim_array.size();
				glm::vec3 last_scale = animation_data.scale_anim_array.back();
				for(int i{}; i<diff; i++)
					animation_data.scale_anim_array.emplace_back( last_scale );
			}
			
		}
	
		//add to array
		std::vector<AnimationDataStruct> bone_animation_array;
		for(int joint_idx : skin.joints)
			bone_animation_array.emplace_back(bone_anim_map[joint_idx]);

		//add to channel map
		bone_animation_channel_map.emplace(anim.name, bone_animation_array);
	}
	
}

//fills in discontinuities in animation timelines (position/rotation/scale) 
void ModelLoader::fillInAnimationGaps(AnimationDataStruct& animation_data){
	//////////////////////////////////
	//TRANSLATION GAPS
	//////////////////////////////////
	//first find indices where gaps occur
	if(!animation_data.translation_anim_array.empty())
	{
		std::map<int, int> gap_idx_trans_array;
		
		int summed_gap_size {};
		for(int y{}; y<animation_data.translation_anim_array.size() - 1; y++){
			float curr_delta = animation_data.trans_time_array[y+1]-animation_data.trans_time_array[y];
			float delta_time = DELTA_TIME_STEP;
			int gap_steps = std::round(curr_delta/delta_time);
			
//			std::cout << y << " " << "ti " << animation_data.trans_time_array[y] << std::endl;
			
			if(curr_delta >= delta_time*1.2f){
//					std::cout << "old index: " << y << ", time " << curr_delta << ", gaps " << gap_steps << ", NEW IDX " << y+summed_gap_size << std::endl;  
				gap_idx_trans_array.emplace(y + summed_gap_size, gap_steps - 1);
				
				summed_gap_size += gap_steps - 1; 
			}
		}
		
		for(const auto& itr : gap_idx_trans_array){
			for(int i{}; i<itr.second; i++){
				glm::vec3 last_pos = animation_data.translation_anim_array[itr.first];
				animation_data.translation_anim_array.insert(animation_data.translation_anim_array.begin() + itr.first + 1, last_pos);
			}
		}
	}
	
	//////////////////////////////////
	//ROTATION GAPS
	//////////////////////////////////
	//first find indices where gaps occur
	if(!animation_data.rotation_anim_array.empty())
	{
		std::map<int, int> gap_idx_rot_array;
		
		int summed_gap_size {};
		for(int y{}; y<animation_data.rotation_anim_array.size() - 1; y++){
			float curr_delta = animation_data.rot_time_array[y+1]-animation_data.rot_time_array[y];
			float delta_time = DELTA_TIME_STEP;
			int gap_steps = std::round(curr_delta/delta_time);
			
//			std::cout << y << " " << "ti " << animation_data.rot_time_array[y] << std::endl;
			
			if(curr_delta >= delta_time*1.2f){
//					std::cout << "old index: " << y << ", time " << curr_delta << ", gaps " << gap_steps << ", NEW IDX " << y+summed_gap_size << std::endl;  
				gap_idx_rot_array.emplace(y + summed_gap_size, gap_steps - 1);
				
				summed_gap_size += gap_steps - 1; 
			}
		}
		
		for(const auto& itr : gap_idx_rot_array){
			for(int i{}; i<itr.second; i++){
				glm::quat last_rot = animation_data.rotation_anim_array[itr.first];
				animation_data.rotation_anim_array.insert(animation_data.rotation_anim_array.begin() + itr.first + 1, last_rot);
			}
		}
	}
	
	
	//////////////////////////////////
	//SCALE GAPS
	//////////////////////////////////
	//first find indices where gaps occur
	if(!animation_data.scale_anim_array.empty())
	{
		std::map<int, int> gap_idx_scale_array;
		
		int summed_gap_size {};
		for(int y{}; y<animation_data.scale_anim_array.size() - 1; y++){
			float curr_delta = animation_data.scale_time_array[y+1]-animation_data.scale_time_array[y];
			float delta_time = DELTA_TIME_STEP;
			int gap_steps = std::round(curr_delta/delta_time);
			
//			std::cout << y << " " << "ti " << animation_data.scale_time_array[y] << std::endl;
			
			if(curr_delta >= delta_time*1.2f){
//					std::cout << "old index: " << y << ", time " << curr_delta << ", gaps " << gap_steps << ", NEW IDX " << y+summed_gap_size << std::endl;  
				gap_idx_scale_array.emplace(y + summed_gap_size, gap_steps - 1);
				
				summed_gap_size += gap_steps - 1; 
			}
		}
		
		for(const auto& itr : gap_idx_scale_array){
			for(int i{}; i<itr.second; i++){
				glm::vec3 last_scale = animation_data.scale_anim_array[itr.first];
				animation_data.scale_anim_array.insert(animation_data.scale_anim_array.begin() + itr.first + 1, last_scale);
			}
		}
	}
	
	
}

float ModelLoader::getAveragedAnimationFps(const std::map<int, AnimationDataStruct>& bone_animation_map){
	//store all timelines for each bones pos/rot/scale in a map, makes it easier to sort and retrieve
	std::map<int, std::vector<float>> sorted_timelines;
	for(const auto& animation_data : bone_animation_map){
		sorted_timelines.emplace(animation_data.second.trans_time_array.size(), animation_data.second.trans_time_array);
		sorted_timelines.emplace(animation_data.second.rot_time_array.size(), animation_data.second.rot_time_array);
		sorted_timelines.emplace(animation_data.second.scale_time_array.size(), animation_data.second.scale_time_array);
	}
	
	//calculate delta time for ALL animation timelines and for each frame, and store inside this array
	std::vector<float> averaged_delta_times_array;
	for(const auto& itr : sorted_timelines){
		const std::vector<float>& timeline = itr.second;
		
		if(timeline.empty())
			continue;
		
		float delta_time {};
		for(int i{}; i<timeline.size() - 1; i++)
			delta_time = timeline[i + 1] - timeline[i];
		averaged_delta_times_array.emplace_back(delta_time);
	}
	
	//if no anims found, loop below prevents crashes 
	if(averaged_delta_times_array.empty())
		return 0.f;
	
	//pick smallest delta time as that should be the value used [better way to do this?]
	std::sort(averaged_delta_times_array.begin(), averaged_delta_times_array.end());
	float smallest_delta = averaged_delta_times_array.front();
	
	//make sure smallest_delta is sensible
	if(smallest_delta == 0.f && averaged_delta_times_array.size() < 2)
		smallest_delta = averaged_delta_times_array[1];
	else{
		PRINT_COLOR("ERROR: delta time cannot be 0", 255, 0, 0);
//		throw std::logic_error("Error");
	}
	
	//make sure it's not large [could imply issues with animation]
	if(smallest_delta > .05f){
		PRINT_WARN("Delta time should not be large for reasonable playback.");
	}
	
	return smallest_delta;
}

std::vector<glm::vec4> ModelLoader::getSkinJoints(const tinygltf::Mesh& mesh){
	std::vector<glm::vec4> joints_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	int joints_idx = primitive.attributes["JOINTS_0"];
	
	//return empy array if no joints found
	if(joints_idx == 0)
		return joints_array;
	
	tinygltf::Accessor& joints_accessor = model.accessors[joints_idx];
	
	//joints
	int byteOffset = model.bufferViews[joints_accessor.bufferView].byteOffset;
	
	if(joints_accessor.byteOffset != 0)
		byteOffset = joints_accessor.byteOffset + byteOffset;
	
	int offset = byteOffset/getSizeOfComponentType(joints_accessor.componentType);
	
	int count = joints_accessor.count;
	
	for(std::size_t i{}; i<count; i++){
		if(joints_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE){
			unsigned char x = uchar_array[(i*4) + 0 + offset];
			unsigned char y = uchar_array[(i*4) + 1 + offset];
			unsigned char z = uchar_array[(i*4) + 2 + offset];
			unsigned char w = uchar_array[(i*4) + 3 + offset];
			joints_array.emplace_back( glm::vec4(x, y, z, w) );
			
//			std::cout << "joint data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
		}
		else if(joints_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
			unsigned short x = ushort_array[(i*4) + 0 + offset];
			unsigned short y = ushort_array[(i*4) + 1 + offset];
			unsigned short z = ushort_array[(i*4) + 2 + offset];
			unsigned short w = ushort_array[(i*4) + 3 + offset];
			joints_array.emplace_back( glm::vec4(x, y, z, w) );
			
//			std::cout << "joint data [x: " << x << ", y: " << y << ", z: " << z << ", w: " << w << "]" << std::endl;
		}
		
	}
	
	return joints_array;
}

std::vector<glm::vec4> ModelLoader::getSkinWeights(const tinygltf::Mesh& mesh){
	
	std::vector<glm::vec4> weights_array;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	int weights_idx = primitive.attributes["WEIGHTS_0"];
	
	//return empy array if no joints found
	if(weights_idx == 0)
		return weights_array;
	
	tinygltf::Accessor& weights_accessor = model.accessors[weights_idx];
	
	//weights
	int byteOffset = model.bufferViews[weights_accessor.bufferView].byteOffset;
	
	if(weights_accessor.byteOffset != 0)
		byteOffset = weights_accessor.byteOffset + byteOffset;
	
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
		throw std::logic_error("Only 1 skin is supported");
	
	tinygltf::Skin& skin = model.skins.front();//.front()
	
	int skin_idx = skin.inverseBindMatrices;
	
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	tinygltf::Accessor& skin_accessor = model.accessors[skin_idx];
	
	//inverse bind matrices [i.e. the bones/their transformation matrices]
	int byteOffset = model.bufferViews[skin_accessor.bufferView].byteOffset;
	
	if(skin_accessor.byteOffset != 0)
		byteOffset = skin_accessor.byteOffset + byteOffset;
	
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

glm::vec3 ModelLoader::getTranslation(const tinygltf::Node& node) const{
	
	glm::vec3 translation = glm::vec3(0.f);
	
	if(!node.translation.empty())
		translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
	
	return translation;
}

glm::quat ModelLoader::getRotation(const tinygltf::Node& node) const{
	
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	
	if(!node.rotation.empty())
		rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
	
	return rotation;
}

glm::vec3 ModelLoader::getScale(const tinygltf::Node& node) const{
	
	glm::vec3 scale = glm::vec3(1.f);
	
	if(!node.scale.empty())
		scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
	
	return scale;
}

glm::mat4 ModelLoader::getTransformMatrix(const tinygltf::Node& node) const{
	
	glm::mat4 matrix = glm::mat4(1.f);
	
	if(!node.matrix.empty()){
		matrix = glm::mat4(
			node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
			node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
			node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
			node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]
			);
	}
	
	return matrix;
}

void ModelLoader::equalizeTRSanimationArrays(AnimationDataStruct& animation_data){
//	std::vector<int> anim_array_sizes = {animation_data.translation_anim_array.size(), animation_data.rotation_anim_array.size(), animation_data.scale_anim_array.size()};
//	std::sort(anim_array_sizes.begin(), anim_array_sizes.end());
	
	std::map<int, std::vector<float>> size_sorted_timelines;
	size_sorted_timelines.emplace(animation_data.trans_time_array.size(), animation_data.trans_time_array);
	size_sorted_timelines.emplace(animation_data.rot_time_array.size(), animation_data.rot_time_array);
	size_sorted_timelines.emplace(animation_data.scale_time_array.size(), animation_data.scale_time_array);
	
	animation_data.time_array = size_sorted_timelines.rbegin()->second;
	
	int max_size = animation_data.time_array.size();
	
	/////////////////////////////
	//equalize translation array
	/////////////////////////////
	if(animation_data.translation_anim_array.size() != max_size){
		//if size 0, then fill will default values
		if(animation_data.translation_anim_array.empty()){
			for(int i{}; i<max_size; i++)
				animation_data.translation_anim_array.emplace_back( glm::vec3(0.f) );
		}
		
		//if few frames short, calc difference and apply
		int diff = max_size - animation_data.translation_anim_array.size();
		glm::vec3 last_pos = animation_data.translation_anim_array.back();
		for(int i{}; i<diff; i++){
			animation_data.translation_anim_array.emplace_back( last_pos );
		}
	}
	
	/////////////////////////////
	//equalize rotations array
	/////////////////////////////
	if(animation_data.rotation_anim_array.size() != max_size){
		//if size 0, then fill will default values
		if(animation_data.rotation_anim_array.empty()){
			for(int i{}; i<max_size; i++)
				animation_data.rotation_anim_array.emplace_back( glm::quat(1.f, 0.f, 0.f, 0.f) );
		}
		
		//if few frames short, calc difference and apply
		int diff = max_size - animation_data.rotation_anim_array.size();
		glm::quat last_rot = animation_data.rotation_anim_array.back();
		for(int i{}; i<diff; i++){
			animation_data.rotation_anim_array.emplace_back( last_rot );
		}
	}
	
	/////////////////////////////
	//equalize scale array
	/////////////////////////////
	if(animation_data.scale_anim_array.size() != max_size){
		//if size 0, then fill will default values
		if(animation_data.scale_anim_array.empty()){
			for(int i{}; i<max_size; i++)
				animation_data.scale_anim_array.emplace_back( glm::vec3(1.f) );
		}
		
		//if few frames short, calc difference and apply
		int diff = max_size - animation_data.scale_anim_array.size();
		glm::vec3 last_pos = animation_data.scale_anim_array.back();
		for(int i{}; i<diff; i++){
			animation_data.scale_anim_array.emplace_back( last_pos );
		}
	}
	
}

std::vector<float> ModelLoader::getTimelineArray(const tinygltf::AnimationSampler& sampler){
	std::vector<float> times;
	{
		int input_idx = sampler.input;
		tinygltf::Accessor time_accessor = model.accessors[input_idx];
		int frame_count = time_accessor.count;
		
		int byteOffset = model.bufferViews[time_accessor.bufferView].byteOffset;
		
		if(time_accessor.byteOffset != 0)
			byteOffset = time_accessor.byteOffset + byteOffset;
		
		int offset = byteOffset/sizeof(float);
		
		for(int t{}; t<frame_count; t++){
			float start_time = float_array[offset];
			float time_ = float_array[t + offset] - start_time; //subtract `start_time` to get it 0 initialized
			times.emplace_back(time_);
		}
	}
	
	return times;
}

bool ModelLoader::isBone(int node_index){
	
	bool result = false;
	
	if(model.skins.empty())
		return result;
	
	tinygltf::Skin& skin = model.skins.front();
	
	std::vector<int> skin_node_indices = skin.joints;
	
//	if( std::find(skin_node_indices.begin(), skin_node_indices.end(), node_index) != skin_node_indices.end() ){
////		std::cout << "bone found: " << model.nodes[node_index].name<< ", index: " << std::distance(skin_node_indices.begin(), skin_node_indices.begin()+node_index) << std::endl;
//		result = true;
//	}

	for(int i : skin_node_indices)
		if(i == node_index)
			return true;
	
	return result;
}

bool ModelLoader::isArmature(int node_index){
	bool result = false;
	
	if(model.skins.empty())
		return result;

	tinygltf::Node& node = model.nodes[node_index];
	
	tinygltf::Skin& skin = model.skins.front();//dont use front
	
	if(model.skins.size() > 1){
		PRINT_COLOR("use an index instead of .front() for getting skin", 255, 0, 0);
	}
	
	//not a good way of checking, but adequate for now since no way to get skin/node index [tinygltf issue?]
	if(node.name == skin.name){
		result = true;
	}
	
	return result;
}

void ModelLoader::getHierarchy(const tinygltf::Node& node_in){
	
//	bool base_reached = false;
	
//	int idx = empty->node_index;
	tinygltf::Node node = node_in;
	
	int tree_level {};
	while(true){
		if(node.children.empty())
			break;
		
		if(isBone(getNodeIndex(node)))
			break;
		
		node = model.nodes[ node.children[0] ];//first child BAD IDEA
		
		//skip if bone
		if(isBone(getNodeIndex(node)))
			break;
		
		tree_level++;
	}
//	std::cout << "levels " <<  tree_level << ", " << node.name << std::endl;
}

int ModelLoader::getNodeIndex(const tinygltf::Node& node){

	for(int i{}; i<model.nodes.size(); i++)
		if(model.nodes[i] == node)
			return i;
	
	return -1;
}

bool ModelLoader::isRootNode(const tinygltf::Node& node){
	
	std::vector<tinygltf::Node> node_with_childs_array;
	std::vector<tinygltf::Node> all_child_nodes_array;
	
	for(const tinygltf::Node& n : model.nodes)
		if(!n.children.empty())
			node_with_childs_array.emplace_back(n);
	
	for(const tinygltf::Node& n : node_with_childs_array){
		for(int sub : n.children)
			all_child_nodes_array.emplace_back(model.nodes[sub]);
	}
	
	const auto is_a_subnode = std::find(all_child_nodes_array.begin(), all_child_nodes_array.end(), node);
		
	if(!node.children.empty() && is_a_subnode == all_child_nodes_array.end())
//		if(!isBone(getNodeIndex(node)) && !isArmature(getNodeIndex(node)))
		return true;
	
	return false;
}

int ModelLoader::getParentNodeIndex(const tinygltf::Node& node){
	
	int node_idx = getNodeIndex(node);
	int parent_idx = -1;
	
	for(tinygltf::Node n : model.nodes){
		
		if(n.children.empty())
			continue;
		
		auto search = std::find( n.children.begin(), n.children.end(), node_idx );
		
		if(search != n.children.end()){
			parent_idx = getNodeIndex(n);
			break;
		}
			
		
	}
	
	
	return parent_idx;
}

std::vector<MeshDataStruct*> ModelLoader::getChildMeshArray(const tinygltf::Node& node){
	std::vector<MeshDataStruct*> child_mesh_array;
	
	for(int c : node.children){
		for(MeshDataStruct* msh : mesh_data_struct_array){
			if(msh->node_index == c)
				child_mesh_array.emplace_back(msh);
		}
	}
	//WARNING - RETURNS A USELESS OBJECT
	return child_mesh_array;
}
