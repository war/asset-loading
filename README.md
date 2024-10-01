
Code Structure
========================

## ModelLoader

The `ModelLoader` class handles the process reading glTF files from disk, and extracting all mesh and animation data required for rendering and animation. It primarly interfaces with the `tinygltf` header for importing of glTF files. All extracted data is stored in separate arrays which can be accessed as public members of the class.

### ModelLoader::ModelLoader constructor
Reading of the glTF file from disk is done through the use of `tinygltf` functions. When a glTF file is specified in binary blob format (.glb), then `tinygltf::TinyGLTF::LoadBinaryFromFile` is called. Otherwise `tinygltf::TinyGLTF::LoadASCIIFromFile` for a plain .gltf extension.
All mesh and animation data (excluding static transforms and hierarchy info) get stored in the `buffer` field of a glTF file. A `buffer` is a long contiguous array of unsigned bytes (8 bit), which must be converted into relevant data types for the model data they represent.  Vertex positions, normals, UV's, skin weights are all floating point data. Vertex indices and skin joint indices can short integers  (2 byte) or stanadard 4 byte integers (sometimes unsigned bytes for joints, if few bones exist/depending on exporter). The buffer byte array is retrieved via `tinygltf::TinyGLTF::Model::buffers`, and is cast into all data types required for the model (float, unsigned short integer, unsigned integer, unsigned byte):
```cpp

//cast buffer byte array into array of floats (for vertex positions and such)
	float_array = (float*)model.buffers.front().data.data();
	
	//cast buffer byte array into array of unsigned shorts (for vertex indices)
	ushort_array = (unsigned short*)model.buffers.front().data.data();
	
		//cast buffer byte array into array of unsigned shorts (for vertex indices if they are large)
	uint_array = (unsigned int*)model.buffers.front().data.data();
	
		//cast buffer byte array into array of unsigned shorts (for skin joint indices)
	uchar_array = (unsigned char*)model.buffers.front().data.data();

```

### ModelLoader::getVertexPositions
Extracts all vertex position data from the cast `float_array` buffer. The `x`, `y` and `z` coordinates of each vertex must be located inside `float_array`. All the required vertex position storage locations and byte offsets for each mesh are sotred in the `accessor` field in a glTF file, which translates to `tinygltf::Accessor` in `tinygltf`. But to get the vertex position `accessor` for each mesh, we must first fetch the `tinygltf::Primitive`, and get the index of the `POSITION` string:
```cpp
//get primitive
	tinygltf::Primitive primitive = mesh.primitives.front();
	//get index used to fetch vertex positions accessor
	int vert_pos_idx = primitive.attributes["POSITION"];

```
`vert_pos_idx` is then used to locate the accessor for this mesh, by using its index as a key in the `tingltf::Model::accessors` array:
```cpp
tinygltf::Accessor vert_pos_accessor = model.accessors[vert_pos_idx];
```
The offset of the vertex position block in the `float_array` can be accessed via the `byteOffset` variable in the `bufferView` class, which is stored in the indexed `tinygltf::::Model::bufferViews` array. This gives the offset size in raw bytes, which will be `= element count x sizeof(float)`. To get the integral offset of each of `x`, `y` and `z` coordinates, we must therefore divide by the sizeof(float). 
```cpp
	int offset = byteOffset/sizeof(float);
```
Finally we can access each vertex position by looping over the total vertex count:
```cpp
	//get vertex positions
	for(std::size_t i{}; i<vertex_count; i++){
		float x = float_array[(i*3) + 0 + offset];
		float y = float_array[(i*3) + 1 + offset];
		float z = float_array[(i*3) + 2 + offset];
	}
```
### ModelLoader::getVertexNormals
Extracts all vertex normal data. The process is exactly the same as what is mentioned in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions). However, the only difference is `NORMAL` string is passed to the `tinygltf::primitive::attributes` array:
```cpp
  //get primitive
	tinygltf::Primitive primitive = mesh.primitives.front();
	//get index used vertex normals accessor
	int vert_norm_idx = primitive.attributes["NORMAL"];
```
### ModelLoader::getVertexUV
Extracts all vertex UV data. The process is exactly the same as what is mentioned in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions). However, `TEXCOORD_0` string is passed to the `tinygltf::TinyGLTF::primitive::attributes` array:
```cpp
  
  //get primitive
	tinygltf::Primitive primitive = mesh.primitives.front();
	//get index used vertex UV accessor
	int vert_uv_idx = primitive.attributes["TEXCOORD_0"];
```
Also, since UV's are only a 2 component vector, we do not need the extra `z` component as found in vertex positions:
```cpp
	//get vertex UV's
	for(std::size_t i{}; i<uvs_count; i++){
		float x = float_array[(i*2) + 0 + offset];
		float y = float_array[(i*2) + 1 + offset];
	}
```
### ModelLoader::getIndices
Extracts all vertex index data. The process is exactly the same as what is mentioned in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions). However, the index used to fetch the `tinygltf::Accessor` is simpler, since it's already stored inside `tinygltf::primitive::indices`. Also, calculating the `offset` may require dividing `byteOffset` by either the `sizeof(unsigned short)` or `sizeof(unsigned int)`. This is automatically handled by the `getSizeOfComponentType` function:
```cpp
	int offset = byteOffset/getSizeOfComponentType(vert_index_accessor.componentType);	
```
Fetching the raw index data is then straightforward, since we directly read values from either `ushort_array` or `uint_array`:
```cpp
	//get vertex indices
	for(std::size_t i{}; i<index_count; i++){
	//if vertex index accessor data is unsigned short 
		if(vert_index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			indices_array.emplace_back( ushort_array[i + offset] );
				//if vertex index accessor data is unsigned int 
		else if(vert_index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			indices_array.emplace_back( uint_array[i + offset] );
	}
```
Note that checks are made to ensure the correct data type and arrays are being used for indices (either unsigned short or int).

### ModelLoader::getSkinWeights
Extracts all skin weight data. The process is exactly the same as what is mentioned in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions).
However, `WEIGHTS_0` string is passed to the `tinygltf::TinyGLTF::primitive::attributes` array:
```cpp
//get primitive
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	//get weight accessor
	int weights_idx = primitive.attributes["WEIGHTS_0"];

```
Since skin weights is a 4 component vector, we must also fetch the additional `w` component:
```cpp
	for(std::size_t i{}; i<weights_accessor.count; i++){
		float x = float_array[(i*4) + 0 + offset];
		float y = float_array[(i*4) + 1 + offset];
		float z = float_array[(i*4) + 2 + offset];
		float w = float_array[(i*4) + 3 + offset];
	}

```

### ModelLoader::getSkinJoints
Extracts all skin joint index data. The process is exactly the same as what is mentioned in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions).
However, `JOINTS_0` string is passed to the `tinygltf::primitive::attributes` array:
```cpp
//get primitive
	tinygltf::Primitive primitive = mesh.primitives.front();
	
	//get joints accessor
	int joints_idx = primitive.attributes["JOINTS_0"];
```
Similar to skin weights, joint indices are also a 4 component vector. However, data type checks are carried out to ensure data from the correct arrays is retrieved (either unsigned byte or unsigned short):
```cpp

	for(std::size_t i{}; i<joints_accessor.count; i++){
	//if joint index data is an unsigned byte
		if(joints_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE){
			unsigned char x = uchar_array[(i*4) + 0 + offset];
			unsigned char y = uchar_array[(i*4) + 1 + offset];
			unsigned char z = uchar_array[(i*4) + 2 + offset];
			unsigned char w = uchar_array[(i*4) + 3 + offset];	
		}
		
		//if joint index data is an unsigned int
		else if(joints_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
			unsigned short x = ushort_array[(i*4) + 0 + offset];
			unsigned short y = ushort_array[(i*4) + 1 + offset];
			unsigned short z = ushort_array[(i*4) + 2 + offset];
			unsigned short w = ushort_array[(i*4) + 3 + offset];
		}
		
	}

```

### ModelLoader::getInverseBindMatrices
Extracts all inverse bind matrix data used in skinning. The process is exactly the same as what is mentioned in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions). However, the index used to fetch the `tinygltf::Accessor` is simpler, since it's already stored inside `tinygltf::Skin::inverseBindMatrices`. The biggest difference here is that there are 16 floating point elements to be extracted per bind matrix, since each matrix is of dimensions 4x4.
```cpp
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
	}

```


## Mesh

#### VAO, VBO, EBO, AnimationPlayer, ModelLoader, default_shader.frag/vert

The `Mesh` class is responsible for rendering and interfacing with the 3D model. Given that some meshes may have skinning animations, it is also responsible for updating skin animations and passing skin weight and index data to the skinned shader. The two key functions of a Mesh class are its constructor, and `Mesh::update()` methods.

### Mesh::Mesh constructor
When a `Mesh` class is created, the relevant mesh data required for rendering and rasterizing the 3D model (vertex coordinates, UV's, etc) are passed to it via the `MeshDataStruct` struct. Although most of this data is already in a usuable format for rendering, other data like tangent and bitangent (required for normal mapping) arrays do not exist yet. Additionally, all this data needs to be layed out in a 1D array with relevant strides and byte offsets demanded by the OpenGL drawing functions. Hence the first course is to create tangent and bitangent arrays, after that we can assemble the data in the required OpenGL format.

The full derivations of tangents and bitangents are not included here for the sake of brevity, but can be found here: https://ogldev.org/www/tutorial26/tutorial26.html
In summary, we loop on a set of 3 vertices (due to it being a triangle), and calculate the displacement and UV vectors of two edges either side of the triangle required for the derived equations:
```cpp

	std::vector<glm::vec3> tangent_array;
	std::vector<glm::vec3> bitangent_array;
	
	std::map<unsigned int, glm::vec3> tangent_map;
	std::map<unsigned int, glm::vec3> bitangent_map;
	
	std::vector<glm::vec3> vert_pos_vec = mesh_data->vertex_positions_array;//store all vertex positions
	std::vector<glm::vec2> uv_coord_vec = mesh_data->vertex_uvs_array;//store all UV's

	for(int i{}; i<mesh_data->vertex_indices_array.size()/3; i++){
		//loop over each set of 3 verts 
		for(int count{}; count<3; count++){
			int vert0_idx = mesh_data->vertex_indices_array[(i*3)+0];
			
			int vert1_idx = mesh_data->vertex_indices_array[(i*3)+1];
			
			int	vert2_idx = mesh_data->vertex_indices_array[(i*3)+2];
			
    //calculate direction vec of 2 edges and their corresponding UV's
    //....
    
    //feed into tangent and bitangent calculation
    //....
    
    //store the result in maps for easy access
			tangent_map.emplace(mesh_data->vertex_indices_array[(i*3) + count], tangent);
			bitangent_map.emplace(mesh_data->vertex_indices_array[(i*3) + count], bitangent);
			
		}
		
	}

```

Now all the data is ready to be filled into `vertex_data_array`. Since the data which will be passed to the shader is layed-out in the order:
```glsl
layout(location = 0) in vec3 vertPos; //vertex pos
layout(location = 1) in vec2 uvVertCoord; //vertex UV's
layout(location = 2) in vec3 vNormal; //vertex normals
layout(location = 3) in vec3 vTangent; //tangent
layout(location = 4) in vec3 vBiTangent; //bitangent
layout(location = 5) in vec4 joints_0; //JOINTS_0
layout(location = 6) in vec4 weights_0; //WEIGHTS_0
```
I.e. (1) vertex positions (2) UV's (3) normals (4) tangent (5) bitangent (6) skin joints (7) skin weights, the data inside `vertex_data_array` must follow the same ordering. Hence we have the following loop:

```cpp
//fill in the vertex_data_array with flattened GLfloat vertex data
	for(unsigned int i{}; i<mesh_data->vertex_positions_array.size(); i++){
	
		glm::vec3& vert_position = mesh_data->vertex_positions_array[i];
		glm::vec2& vert_uv = mesh_data->vertex_uvs_array[i];
		glm::vec3& vert_normal = mesh_data->vertex_normals_array[i];
		glm::vec3& tangent = tangent_array[i];
		glm::vec3& bitangent = bitangent_array[i];
		
		//push vert positions
		vertex_data_array.emplace_back( (GLfloat)vert_position.x );		vertex_data_array.emplace_back( (GLfloat)vert_position.y );	vertex_data_array.emplace_back( (GLfloat)vert_position.z );
		
		//push vert uvs
		vertex_data_array.emplace_back( (GLfloat)vert_uv.x );		vertex_data_array.emplace_back( (GLfloat)vert_uv.y );
		
		//push vert normals
		vertex_data_array.emplace_back( (GLfloat)vert_normal.x );		vertex_data_array.emplace_back( (GLfloat)vert_normal.y );		vertex_data_array.emplace_back( (GLfloat)vert_normal.z );
		
		//tang
		vertex_data_array.emplace_back( tangent.x );		vertex_data_array.emplace_back( tangent.y );		vertex_data_array.emplace_back( tangent.z );
		
		//bitang
		vertex_data_array.emplace_back( bitangent.x );		vertex_data_array.emplace_back( bitangent.y );		vertex_data_array.emplace_back( bitangent.z );
		
		//only if it has skinning
		if(mesh_data->has_skin){
			//joints
			{
				glm::vec4 joint = mesh_data->joints_array[i];
				
				vertex_data_array.emplace_back( joint.x );				vertex_data_array.emplace_back( joint.y );				vertex_data_array.emplace_back( joint.z );				vertex_data_array.emplace_back( joint.w );
			}
			//weights
			{
				glm::vec4 weight = mesh_data->weights_array[i];
				
				vertex_data_array.emplace_back( weight.x );				vertex_data_array.emplace_back( weight.y );				vertex_data_array.emplace_back( weight.z );				vertex_data_array.emplace_back( weight.w );
			}
		}
	
	}

```
So for each vertex, its data (position, UV, etc) is pushed into `vertex_data_array` into the form `{x_position, y_position, z_position,       uv_x, uv_y,       ....}`.

Since OpenGL relies on the usage of abstract data buffers for rendering (VAO, VBO, EBO), we must transfer the raw data stored in `vertex_data_array` into these buffers. First, we contruct and bind the VAO, VBO and EBO:
```cpp
	vao.bind(); //VAO has already been constructed on init of Mesh class
	vbo = VBO(vertex_data_array); //construct VBO
	ebo = EBO(vertex_indices_array); //construct EBO
```
Now we must setup pointers and define the strides and byte offsets of our vertex coordinates, UV, etc data stored within `vertex_data_array`. Vertex position, UV, normal, tangent, bitangent, skin joint index and skin weights will each have pointers. 

```cpp
		//vertex position pointer (3 component vec)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), static_cast<void*>(0));
		glEnableVertexAttribArray(0);
		
		//vertex UV pointer (2 component vec)
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 3*sizeof(float) ));
		glEnableVertexAttribArray(1);
		
		//vertex normal pointer (3 component vec)
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 5*sizeof(float) ));
		glEnableVertexAttribArray(2);
		
		//vertex tangent pointer (3 component vec)
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 8*sizeof(float) ));
		glEnableVertexAttribArray(3);
		
		//vertex bitangent pointer (3 component vec)
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 11*sizeof(float) ));
		glEnableVertexAttribArray(4);
		
		//skin joints pointer (4 component vec)
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 14*sizeof(float) ));
		glEnableVertexAttribArray(5);
		

    //skin weights pointer (4 component vec)
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 22*sizeof(float), reinterpret_cast<void*>( 18*sizeof(float) ));
		glEnableVertexAttribArray(6);	

```
Note that the skin joints index and skin weights are both 4 component vectors. While vertex position, normal, tangent and bitangent are 3 components. And UV is only 2 components. Hence the length of a single vertex data block (or stride) in our setup is 22 (2 + 3 + 3 + 3 + 3 + 4 + 4). And since we are using floating point data, the total size will be 22*sizeof(float) = 22*4 = 88.
Once we have finished setting up pointers, we must unbind VAO, VBO and EBO to avoid modifying them and corrupting the data.

```cpp
	vbo.unbind();
	vao.unbind();
	ebo.unbind();
```

### Mesh::update()
The `Mesh::update()` function is called every frame of the application. I.e. in Main.cpp:
```cpp

int main(int argc, char* argv[]) {
    Mesh* mesh = ....;
    
    //update loop
    while(!windowManager.shouldClose()){
    
        //runs every frame
        mesh->update()
    }

    return 0;
}

```
This function primarily handles the rendering side (including sending uniform data to shaders), but also updating the mesh transforms (position, rotation, scale), and in case of skinning, updating skinning animations.
Before asking OpenGL to render a mesh (achieved via `glDrawElements`), we must first pass all the required uniforms to the shader. This includes camera matrix and position, mesh transforms, directional light info,  textures, and skinning data.
Sending camera and mesh matrices:
```cpp
	shader->setMat4("modelMatrix", modelMatrix);//pass mesh model matrix (the matrix which defines the meshes' position, rotation and scale)
	shader->setMat4("viewMatrix", camera->GetViewMatrix());//camera view matrix
	shader->setMat4("projMatrix", glm::perspective(camera->FovRads, window_manager->getAspectRatio(), 0.1f, 1000.0f));//camera projection matrix
	shader->setVec3("cameraPos", camera->GetPosition());//camera positon

```
Sending directional lighting data:
```cpp
//pass light info
	shader->setFloat("light_strength", direct_light->strength);//strength
	shader->setFloat("light_specular", direct_light->specular);//specularity
	shader->setVec3("light_color", direct_light->color);//col
	shader->setVec3("light_dir", direct_light->direction);//normalized direction

```
Sending texture GLuint indices. For GL textures, `glActiveTexture(GL_TEXTURE_2D, GL_TEXTURE_ID)` must first be called, followed by `glBindTexture(GL_TEXTURE<INDEX>)`.
```cpp
	//diffuse tex
	if(mesh_data->texture_map.count(TextureType::DIFFUSE) != 0){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh_data->texture_map[TextureType::DIFFUSE].tex_id);
		shader->setInt("diffuse_tex", 0);//send Image to frag shader
	}
	
	//normal tex
	if(mesh_data->texture_map.count(TextureType::NORMAL) != 0){
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mesh_data->texture_map[TextureType::NORMAL].tex_id);
		shader->setInt("normal_tex", 1);//send Image to frag shader
	}
	
	//metal tex
	if(mesh_data->texture_map.count(TextureType::METAL) != 0){
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mesh_data->texture_map[TextureType::METAL].tex_id);
		shader->setInt("metal_tex", 2);//send Image to frag shader
	}
```
The diffuse (albedo) texture is sent first, hence the <INDEX> must be 0, i.e.
`glActiveTexture(GL_TEXTURE0)`. The `GL_TEXTURE_ID` used in `glBindTexture` can retrieved from the `texture_map` stored in `MeshDataStruct`, which fetches the relevant diffuse/metal/normal texture based on `TextureType` key supplied. I.e. `MeshDataStruct::texture_map[TextureType::DIFFUSE].tex_id`.

Base material color is also optionally sent to the shader as well. This will modify the final color of the mesh after the albedo texture is applied. However, it can be set to plain white `glm::vec3(1.f)` if not used.
```cpp
	//send base/diffuse color
	shader->setVec3("base_color", glm::vec3( mesh_data->material_data.base_color.x, mesh_data->material_data.base_color.y, mesh_data->material_data.base_color.z ) );
	
```

Since skinned meshes require inverse bind matrices to be used in realtime, these are also passed to the shader via the `inverseBindMatrixArray` uniform:
```cpp
	//send skinned matrices to shader
	for(short m{}; m<bone_skinned_matrix_array.size(); m++){
		std::string uniform_name = std::string("inverseBindMatrixArray[") + std::to_string(m) + std::string("].matrix");
		shader->setMat4(uniform_name.c_str(), bone_skinned_matrix_array[m]);
	}
	
	//send isSkinned var
	shader->setInt("isSkinned", (int)mesh_data->has_skin);

```
The `inverseBindMatrixArray` is an array of fixed (and maximum) size 64. These means that any skinned mesh can only have 64 bones at a time. This fixed value can be increased to, say, 128. But due to OpenGL array size limitations, this may not work on all graphics cards. The boolean value of `isSkinned` is also sent, and if the mesh has no skinning, then it will discard the skinning calculation loop.

Finally, to render the mesh, `glDrawElements` is called.
```cpp
//render
	glDrawElements(render_mode, vertex_indices_array.size(), GL_UNSIGNED_INT, 0);//rendering part
	
```
The `render_mode` variable is a GLenum, which can also be set externally by calling `Mesh::setRenderingMode(GLenum)`. By default, solid filled triangles are rendered using `GL_TRIANGLES`. But lines (`GL_LINES`) and points (`GL_POINTS`) can also be chosen if so desired.
The number of indices is also a requirement for indexed rendering, thus the .size()` of the `vertex_indices_array` is passed as the second argument.


### Mesh::updateSkinnedAnimation()
Updating all skinning matrices, including calculating bone transforms every frame is carried out in this function.

Since bones are chained together, and each child bones' transform depends on its root bones' transform, we must find the root of each corresponsing child bone, and multiply their transforms in correct order to get the final child bone pose.
Once the bone position, rotation and scale is calculated for the current frame, and a transform matrix is created, this is multiplied by the parent bone transform to get the final pose of the bone.
```cpp
    //get position
		glm::vec3 bone_pos = calculateCurrentTranslation(bone_anim);
		//get rotation
		glm::quat bone_rot = calculateCurrentRotation(bone_anim);
		//get scale
		glm::vec3 bone_scale = calculateCurrentScale(bone_anim);
		
		//create transform matrix
		glm::mat4 bone_transform = createTRSmatrix( bone_pos, bone_rot, bone_scale );
		
				//check if it has a root, then multiply the current child pose by root pose
		bone_nodes_vec.emplace_back(bone_anim.node_index);
		if (bone_anim.has_root) {
			auto bone_idx_itr = std::find(bone_nodes_vec.begin(), bone_nodes_vec.end(), bone_anim.root_idx);
			if ( bone_idx_itr != bone_nodes_vec.end() ) {
				std::size_t root_mat_index = std::distance(bone_nodes_vec.begin(), bone_idx_itr);
				bone_transform = bone_transform_matrix_array[ root_mat_index ] * bone_transform;
			}
		}

```
To get the final transform matrix which will be sent and directly used in the shader, the inverse bind matrix for each bone must be multipled by the transposed tranform matrix, and transposed one final time:
```cpp

	for (std::size_t m{}; m < inverse_bind_mat_array.size(); m++) {
		
		//get inverse bind matrix for each bone
		glm::mat4 inverse_bind_matrix = inverse_bind_mat_array[m]
		
		//multiply inverse bind matrix by trasnposed transform matrix
		glm::mat4 skinned_mat = inverse_bind_matrix * glm::transpose( bone_transform_matrix_array[m] );
		//transpose resultant matrix one final time before adding to the array (which will be dispatched to shader)
		bone_skinned_matrix_array.emplace_back( glm::transpose( skinned_mat ) );
	}

```
This completes the skinning aspect, and provides us with matrices which are ready to be used in the shader.


### Misc functions

`Mesh::setModelMatrix(glm::mat4)` - Used to directly set the model matrix of the mesh (with all of position, rotation and scale transforms applied)

`Mesh::enableBackFaceCulling(bool)` - Enable/disable backface culling

`Mesh::setSkinnedAnimationChannel(std::string)` - Set name of animation channel for playback on skinned mesh (in case multiple channels exist)









# fps
model -> res/models/pistol/scene.gltf

# dependancies
git  
cmake

# basic run-down
Camera.cpp - handles the camera  
Main.cpp - contains main while loop, sets up the model, map, viewport etc  
Map.cpp - creates and draws grid in the level  
ModelLoader.cpp - contains the initial loading of the model (tinygltf::model)  
Shader.cpp - contains basic shader loading/using system  
Utils/GLMUtils.cpp - contains to_string capability for glm::vec2, glm::vec3, glm::vec4, glm::quat (for debugging)  
Window/WindowManager.cpp - handles the main SDL2 window, fps counter, and inputs

# inputs
WASD - move forward, left, down, right  
EQ - move up and down (y axis)  
SHIFT - draw models in wireframe mode  
CTRL - increased speed when held (affects WASD & EQ)
