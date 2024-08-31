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
		throw std::logic_error("Buffer must not be empty.");
	
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
		
//		std::cout << "node idx " << mesh_node_idx << ", name " << mesh.name << ", node name : " << node.name << std::endl; 
		
		MeshDataStruct mesh_data_struct {};//will store all mesh data [vertex pos/norm/uv, global mesh pos/rot etc]
		
		mesh_data_struct.name	= mesh.name;
		
		if(mesh.primitives.size() > 1)
			throw std::logic_error("Only 1 primitive per mesh is supported");
		//check to see if has sub-nodes
		if(!node.children.empty()){
			mesh_data_struct.has_childs = true;
			mesh_data_struct.childs_array = node.children;
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
	
		///////////////
		//TEXTURES
		///////////////
		mesh_data_struct.texture_map = getTextureMap(mesh);
		
		///////////////////////////
		//GLOBAL POS/ROT/SCALE
		///////////////////////////
		{
			//find node this mesh is assigned to
			mesh_data_struct.node_index = mesh_node_idx;
			if(!node.translation.empty())//translation
				mesh_data_struct.translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
			if(!node.rotation.empty())//rotation
				mesh_data_struct.rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			if(!node.scale.empty())//scale
				mesh_data_struct.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
			
		}
		
		/*
		if(mesh.name == "lid_Mat_0"){
			PRINT_WARN("###################");
			PRINT_WARN("name " + mesh.name);
			for(auto v : mesh_data_struct.vertex_positions_array)
				printGlmVec3(v);
			PRINT_WARN("###################");
		}
		
		*/
		
		///////////////////////////
		//SKINNING [JOINTS/WEIGHTS]
		///////////////////////////
		if(!model.skins.empty()){//need better checks than this
			
			has_skin = true;//USELESS --REMOVE THIS once animation for skinning reworked [needed rn as used by animations]
			
			mesh_data_struct.has_skin = true;
			
			//joints
			mesh_data_struct.joints_array = getSkinJoints(mesh);
			
			//weights
			mesh_data_struct.weights_array = getSkinWeights(mesh);
			
			if(mesh_data_struct.joints_array.empty() || mesh_data_struct.weights_array.empty()){
				has_skin = false;
				mesh_data_struct.has_skin = false;
			}
		
			//inverse bind matrices
			mesh_data_struct.inverse_bind_matrix_array = getInverseBindMatrices(mesh);
		}
		
		///////////////////////////
		//ANIMATIONS
		///////////////////////////
		mesh_data_struct.animation_data = getMeshAnimationData(mesh);
		
		/////////////////
		//MATERIALS
		/////////////////
		mesh_data_struct.material_data = getMaterial(mesh);
		
		//add to MeshDataStruct array
		mesh_data_struct_array.emplace_back(mesh_data_struct);
	}
	
	/*
	///////////////////
	//LOAD ALL EMPTIES
	///////////////////
	*/
	
//	getAllNodeAnimationTimelines();
	
//	std::vector<float> max_node_timeline = node_timelines_map.rbegin()->second;
//	
	
	for(int n{}; n<model.nodes.size(); n++){
		Empty empty;
		
		tinygltf::Node& node = model.nodes[n];
		
		empty.node_index = n;
		
		empty.name = node.name;
		
		
		//checks to make sure this is NOT a mesh
		if(node.mesh != -1){
			continue;
		}
		//checks to make sure this is NOT a bone
		if( isBone(empty.node_index) ){
			continue;
		}
		//checks to make sure this is NOT an armature
		if( isArmature(empty.node_index) ){
			continue;
		}
		
		//check to see if has sub-nodes
		if(!node.children.empty()){
			empty.has_childs = true;
			empty.child_array = node.children;
		}
		
		///////////////////////////
		//GLOBAL POS/ROT/SCALE
		///////////////////////////
		{
			//find node this mesh is assigned to
			if(!node.translation.empty())//translation
				empty.position = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
			if(!node.rotation.empty())//rotation
				empty.rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			if(!node.scale.empty())//scale
				empty.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
			
		}
		
		/////////////
		//ANIMATIONS
		/////////////
		empty.animation_data = getNodeAnimationData(node);
		
		//add to array
		empties_array.emplace_back(empty);
	}
	
	
	////////////////
	//get skinned anims
	////////////////
	getSkinnedAnimation();
//	GET_SKINNED_ANIMATION_BLENDER();
	
}

ModelLoader::~ModelLoader() {
	
	//delete all GL texture's once finished
	for(const MeshDataStruct& mesh_data : mesh_data_struct_array){
		for(const auto& tex_data : mesh_data.texture_map){
			glDeleteTextures(1, &tex_data.second.tex_id);
		}
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
//		throw std::logic_error("Translation, scale and rotation animation durations must be equal.");
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
		std::vector<float> times = getTimelineArray(time_sampler);
		
		tinygltf::AnimationChannel& channel = animation.channels[c];
		tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
		
		int input_idx = channel.target_node;
		int output_idx = sampler.output;
		
		
		//detect different animations within the same channel
		if(input_idx != node_idx){
			continue;
		}
		animation_data.has_animation = true;
		
		animation_data.time_array = times;
		
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
	
	//adds check to ensure all arrays are equal [will be fixed soon]
	if( animation_data.translation_anim_array.size() != animation_data.rotation_anim_array.size() || animation_data.translation_anim_array.size() != animation_data.scale_anim_array.size() || animation_data.rotation_anim_array.size() != animation_data.scale_anim_array.size() ){
		equalizeTRSanimationArrays(animation_data);
//		PRINT_WARN("Translation, scale and rotation animation durations must be equal.");
//		throw std::logic_error("Translation, scale and rotation animation durations must be equal.");
	}
	
	
	return animation_data;
}

AnimationDataStruct ModelLoader::getBLENDER_NODE_ANIMATION_DATA(const tinygltf::Node& node){
	
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
		std::vector<float> times = getTimelineArray(time_sampler);
		
		tinygltf::AnimationChannel& channel = animation.channels[c];
		tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
		
		int input_idx = channel.target_node;
		int output_idx = sampler.output;
		
		
		//detect different animations within the same channel
		if(input_idx != node_idx){
			continue;
		}
		animation_data.has_animation = true;
		
		animation_data.time_array = times;
		
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
			}
			
			//scale
			if(target_path == "scale"){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				
				animation_data.scale_time_array = getTimelineArray(sampler);
			}
			
		}
		
		
	}
	
	//adds check to ensure all arrays are equal [will be fixed soon]
	if( animation_data.translation_anim_array.size() != animation_data.rotation_anim_array.size() || animation_data.translation_anim_array.size() != animation_data.scale_anim_array.size() || animation_data.rotation_anim_array.size() != animation_data.scale_anim_array.size() ){
		equalizeTRSanimationArrays(animation_data);
//		PRINT_WARN("Translation, scale and rotation animation durations must be equal.");
//		throw std::logic_error("Translation, scale and rotation animation durations must be equal.");
	}
	
	
	return animation_data;
}

void ModelLoader::getSkinnedAnimation(){
	
	if(model.skins.empty())
		return;
	
	tinygltf::Skin& skin = model.skins.front();
	
	std::vector<int> skin_node_indices = skin.joints;
	
	///////////////////////////
	//ANIMATIONS
	///////////////////////////
	if(model.animations.empty()){
		return;
	}
		
	
	//more than 1 animation?
	tinygltf::Animation anim;
	
	//find animation for this skin
	for(const tinygltf::Animation& a : model.animations){
		for(const tinygltf::AnimationChannel& c : a.channels){
			if(std::find(skin_node_indices.begin(), skin_node_indices.end(), c.target_node) != skin_node_indices.end()){
				anim = a;
				break;
			}
		}
	}
	
	
	animation_name = anim.name;
	
	/*
	for(auto s : skin_node_indices)
		PRINT_COLOR("BONE " + std::to_string(s) + "  " + model.nodes[s].name, 0, 0, 255);
	*/
	
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
		std::vector<float> times = getTimelineArray(time_sampler);
		
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
	
	//get max timeline
	std::vector<float> max_time_array = timeline_map.rbegin()->second;
	
	//add anim for each bone [will be filled in later]
	for(int joint_idx : skin.joints){
		AnimationDataStruct animation_data;
		animation_data.node_index = joint_idx;
		animation_data.time_array = max_time_array;
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
				
				animation_data.time_array = max_time_array;
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
				
				animation_data.time_array = max_time_array;
			}
			
			//scale
			if(target_path == "scale"){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
	//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				
				animation_data.scale_time_array = getTimelineArray(sampler);
				
				animation_data.time_array = max_time_array;
			}
		
	}
	
}
	
	
	//get static translations/rots/scales for each bone
	for(auto& itr : bone_anim_map){
		AnimationDataStruct& anim = itr.second;
		const tinygltf::Node& bone_node = model.nodes[ anim.node_index ];
		
		//trans
		if(!bone_node.translation.empty()){
			anim.translation = glm::vec3( bone_node.translation[0], bone_node.translation[1], bone_node.translation[2] ) ;
		}
		//rot
		if(!bone_node.rotation.empty()){
			anim.rotation = glm::quat( bone_node.rotation[3], bone_node.rotation[0], bone_node.rotation[1], bone_node.rotation[2] ) ;
		}
		//scale
		if(!bone_node.scale.empty()){
			anim.scale = glm::vec3( 1.f ) ;
		}
	}
	
	//
	for(auto& v : bone_anim_map){
		if(!v.second.has_animation){
			AnimationDataStruct& anim = v.second;
			anim.has_animation = true;
//			PRINT_COLOR("bone with no anim data -> " + model.nodes[anim.node_index].name,255,0,0);
		}
	}
	
	//find root bones 
	for(auto& v : bone_anim_map){
		AnimationDataStruct& anim = v.second;
		for(std::size_t n{}; n<model.nodes.size(); n++){
			std::vector<int> childs_vec = model.nodes[n].children;
			if( std::find(childs_vec.begin(), childs_vec.end(), anim.node_index) != childs_vec.end() ){
				anim.has_root = true;
				anim.root_idx = n;
				
				break;
			}
			
		}
	}
	
	//equalize empty arrays
	for(auto& v : bone_anim_map){
		AnimationDataStruct& anim = v.second;
		
		//pos
		if(anim.translation_anim_array.empty())
			for(auto t : max_time_array)
				anim.translation_anim_array.emplace_back(anim.translation);
		
		//rots
		if(anim.rotation_anim_array.empty())
			for(auto t : max_time_array)
				anim.rotation_anim_array.emplace_back(anim.rotation);
		
		//scales
		if(anim.scale_anim_array.empty())
			for(auto t : max_time_array)
				anim.scale_anim_array.emplace_back(anim.scale);
			
	}
	
	//match up array sizes
	for(auto& v : bone_anim_map){
		AnimationDataStruct& anim = v.second;
		
		int max_size = anim.time_array.size();
		
		//equalize translation array
		if(anim.translation_anim_array.size() != max_size){
			//if few frames short, calc difference and apply
			int diff = max_size - anim.translation_anim_array.size();
			glm::vec3 last_pos = anim.translation_anim_array.back();
			for(int i{}; i<diff; i++)
				anim.translation_anim_array.emplace_back( last_pos );
		}
		
		//equalize rot array
		if(anim.rotation_anim_array.size() != max_size){
			//if few frames short, calc difference and apply
			int diff = max_size - anim.rotation_anim_array.size();
			glm::quat last_rot = anim.rotation_anim_array.back();
			for(int i{}; i<diff; i++)
				anim.rotation_anim_array.emplace_back( last_rot );
		}
		
		//equalize scale array
		if(anim.scale_anim_array.size() != max_size){
			//if few frames short, calc difference and apply
			int diff = max_size - anim.scale_anim_array.size();
			glm::vec3 last_scale = anim.scale_anim_array.back();
			for(int i{}; i<diff; i++)
				anim.scale_anim_array.emplace_back( last_scale );
		}
		
	}
	
	//add to array
	for(auto& v : bone_anim_map){
		AnimationDataStruct& anim = v.second;
		
		/*
		if(model.nodes[anim.node_index].name == "Joint_3_06"){
			PRINT_WARN("ROOT ->" + model.nodes[anim.root_idx].name);
		}
		
		PRINT_WARN("##############");
		std::cout << "time len " << anim.time_array.size()<<std::endl;
		std::cout << "pos len " << anim.translation_anim_array.size()<<std::endl;
		std::cout << "rot len " << anim.rotation_anim_array.size()<<std::endl;
		std::cout << "sca len " << anim.scale_anim_array.size()<<std::endl;
		*/
		
		bone_animation_array.emplace_back(anim);
	}
	
	
	
}

void ModelLoader::GET_SKINNED_ANIMATION_BLENDER(){
	
	if(model.skins.empty())
		return;
	
	tinygltf::Skin& skin = model.skins.front();
	
	std::vector<int> skin_node_indices = skin.joints;
	
	///////////////////////////
	//ANIMATIONS
	///////////////////////////
	if(model.animations.empty()){
		return;
	}
	
	
	//more than 1 animation?
	tinygltf::Animation anim;
	
	//find animation for this skin
	for(const tinygltf::Animation& a : model.animations){
		for(const tinygltf::AnimationChannel& c : a.channels){
			if(std::find(skin_node_indices.begin(), skin_node_indices.end(), c.target_node) != skin_node_indices.end()){
				anim = a;
				break;
			}
		}
	}
	
	animation_name = anim.name;
	
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
		
		/////////////////////
		//fetch times
		/////////////////////
		tinygltf::AnimationSampler& time_sampler = anim.samplers[0];
		std::vector<float> times = getTimelineArray(time_sampler);
		
		
		
		
		
		///////////////////////
		//fetch translations/rots/scale
		///////////////////////
		/*
		PRINT_WARN("targ node " + std::to_string(node_idx) + ", name " + model.nodes[node_idx].name);
		
		//keep track of all bone indices
		if( std::find(all_bone_indices_in_array.begin(), all_bone_indices_in_array.end(), node_idx) == all_bone_indices_in_array.end() ){
//			PRINT_WARN("zzzzzzzzzzzzzzzzzzzzzz");
		all_bone_indices_in_array.emplace_back(node_idx);
		}
		if(all_bone_indices_in_array.empty())
		all_bone_indices_in_array.emplace_back(node_idx);
		*/
		
		
		///////////////////
		///////ADDING TO ARRAY
		if(idx != node_idx){
			bone_animation_array.emplace_back(AnimationDataStruct{}); 
			idx = node_idx;
//			PRINT_COLOR("new bone added " + std::to_string(idx) + "  " + model.nodes[idx].name, 255, 0, 0);
		}
		if(bone_animation_array.empty())
			bone_animation_array.emplace_back(AnimationDataStruct{}); 
		
		AnimationDataStruct& animation_data = bone_animation_array.back();//CHANGE THIS TO USE CURR INDEX AND NOT LAST EELEMNT
		
		animation_data.time_array = times;
		
		animation_data.has_animation = true;
		animation_data.node_index = node_idx;
		/////////////////////////////////////
		/////////////////////////////////////
		
		
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
				
			}
			
			//scale
			if(target_path == "scale"){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				animation_data.scale_anim_array.emplace_back( glm::vec3(x, y, z) );
//					std::cout << "scale data [x: " << x << ", y: " << y << ", z: " << z << "]" << std::endl;
				
				animation_data.scale_time_array = getTimelineArray(sampler);
				
			}
			
		}
		
		
	}
	
	
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
	
	if( std::find(skin_node_indices.begin(), skin_node_indices.end(), node_index) != skin_node_indices.end() ){
//		std::cout << "bone found: " << model.nodes[node_index].name<< ", index: " << std::distance(skin_node_indices.begin(), skin_node_indices.begin()+node_index) << std::endl;
		result = true;
	}
	
	return result;
}

bool ModelLoader::isArmature(int node_index){
	bool result = false;
	
	if(model.skins.empty())
		return result;

	tinygltf::Node& node = model.nodes[node_index];
	
	tinygltf::Skin& skin = model.skins.front();
	
	//not a good way of checking, but sufficient for now since no way to get skin/node index [tinygltf issue?]
	if(node.name == skin.name){
		result = true;
	}
	
	return result;
}
