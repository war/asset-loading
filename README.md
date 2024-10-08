Code Structure Breakdown
=====================

## Table of Contents
========================

[Main](Code%20Structure%20Breakdown.md#Main)


[ModelLoader](Code%20Structure%20Breakdown.md#ModelLoader)

- [ModelLoader::ModelLoader (constructor)](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AModelLoader%20%28constructor%29)

- [ModelLoader::~ModelLoader (destructor)](Code%20Structure%20Breakdown.md#ModelLoader%3A%3A~ModelLoader%20%28destructor%29)

- [ModelLoader::getVertexPositions](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetVertexPositions)

- [ModelLoader::getVertexNormals](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetVertexNormals)

- [ModelLoader::getVertexUV](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetVertexUV)

- [ModelLoader::getIndices](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetIndices)

- [ModelLoader::getSkinWeights](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetSkinWeights)

- [ModelLoader::getSkinJoints](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetSkinJoints)

- [ModelLoader::getInverseBindMatrices](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetInverseBindMatrices)

- [ModelLoader::getTextureMap](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetTextureMap)

- [ModelLoader::getMaterial](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetMaterial)

- [ModelLoader::fillInAnimationGaps](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AfillInAnimationGaps)

- [ModelLoader::equalizeAndMatchNodeAnimations](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AequalizeAndMatchNodeAnimations)

- [ModelLoader::getTimelineArray](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetTimelineArray)

- [ModelLoader::getMaxNodeTimeline](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetMaxNodeTimeline)

- [ModelLoader::getNodeAnimationData](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetNodeAnimationData)

- [ModelLoader::getMeshAnimationData](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetMeshAnimationData)

- [ModelLoader::getTranslation](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetTranslation)

- [ModelLoader::getRotation](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetRotation)

- [ModelLoader::getScale](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetScale)

- [ModelLoader::getTransformMatrix](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetTransformMatrix)

- [ModelLoader::getMaxSkinnedDuration](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetMaxSkinnedDuration)

- [ModelLoader::getSkinnedAnimation](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetSkinnedAnimation)

- [ModelLoader::getParentNodeIndex](Code%20Structure%20Breakdown.md#ModelLoader%3A%3AgetParentNodeIndex)


[Mesh](Code%20Structure%20Breakdown.md#Mesh)

- [Mesh::Mesh constructor](Code%20Structure%20Breakdown.md#Mesh%3A%3AMesh%20constructor)

- [Mesh::update()](Code%20Structure%20Breakdown.md#Mesh%3A%3Aupdate%28%29)

- [Mesh::updateSkinnedAnimation()](Code%20Structure%20Breakdown.md#Mesh%3A%3AupdateSkinnedAnimation%28%29)


[AnimationPlayer](Code%20Structure%20Breakdown.md#AnimationPlayer)

- [AnimationPlayer::AnimationPlayer (constructor)](Code%20Structure%20Breakdown.md#AnimationPlayer%3A%3AAnimationPlayer%20%28constructor%29)

- [AnimationPlayer::update](Code%20Structure%20Breakdown.md#AnimationPlayer%3A%3Aupdate)

- [AnimationPlayer::calculateCurrentTranslation](Code%20Structure%20Breakdown.md#AnimationPlayer%3A%3AcalculateCurrentTranslation)

- [AnimationPlayer::calculateCurrentRotation](Code%20Structure%20Breakdown.md#AnimationPlayer%3A%3AcalculateCurrentRotation)

- [AnimationPlayer::calculateCurrentScale](Code%20Structure%20Breakdown.md#AnimationPlayer%3A%3AcalculateCurrentScale)


[VAO](Code%20Structure%20Breakdown.md#VAO)

- [VAO::VAO (constructor)](Code%20Structure%20Breakdown.md#VAO%3A%3AVAO%20%28constructor%29)

- [VAO::free](Code%20Structure%20Breakdown.md#VAO%3A%3Afree)

- [VAO::bind](Code%20Structure%20Breakdown.md#VAO%3A%3Abind)

- [VAO::unbind](Code%20Structure%20Breakdown.md#VAO%3A%3Aunbind)


[VBO](Code%20Structure%20Breakdown.md#VBO)

- [VBO::VBO (constructor)](Code%20Structure%20Breakdown.md#VBO%3A%3AVBO%20%28constructor%29)

- [VBO::free](Code%20Structure%20Breakdown.md#VBO%3A%3Afree)

- [VBO::bind](Code%20Structure%20Breakdown.md#VBO%3A%3Abind)

- [VBO::unbind](Code%20Structure%20Breakdown.md#VBO%3A%3Aunbind)


[EBO](Code%20Structure%20Breakdown.md#EBO)

- [EBO::EBO (constructor)](Code%20Structure%20Breakdown.md#EBO%3A%3AEBO%20%28constructor%29)

- [EBO::free](Code%20Structure%20Breakdown.md#EBO%3A%3Afree)

- [EBO::bind](Code%20Structure%20Breakdown.md#EBO%3A%3Abind)

- [EBO::unbind](Code%20Structure%20Breakdown.md#EBO%3A%3Aunbind)


[default_shader](Code%20Structure%20Breakdown.md#default_shader)

- [default_shader.vert (vertex shader)](Code%20Structure%20Breakdown.md#default_shader.vert%20%28vertex%20shader%29)

- [default_shader.frag (fragment shader)](Code%20Structure%20Breakdown.md#default_shader.frag%20%28fragment%20shader%29)

## Main
========================

Loading of glTF meshes is done by creating an instance of the `ModelLoader` class inside the `main` function:

```cpp

int main(int argc, char* argv[]){
    
    //load model
    ModelLoader* model = new ModelLoader("res/models/pistol/scene.gltf");

}

```
The `Mesh` objects are also created by creating an instance of the  `Mesh` class, and stored inside `mesh_array` array for easy access. However, since more than one mesh may exist in a glTF model, we must loop over the all `MeshDataStruct` objects stored in the loaded `ModelLoader` class, and create a new instance of `Mesh` every time, emplacing each in the `mesh_array`:

```cpp

int main(int argc, char* argv[]){
    
    //load model
    ModelLoader* model = new ModelLoader("res/models/pistol/scene.gltf");

		//spawn meshes
		for(MeshDataStruct* mesh_data : model->getMeshDataArray()){
		
			Mesh* mesh = new Mesh(&camera, model, mesh_data, &defaultShader, &windowManager, direct_light);
			
			mesh_array.emplace_back(mesh);
		}
}

```
Since certain skinned animations may contain more than one channel/cycle, the cycle to be played can be specified per `Mesh` by specifying the cycle/channel name:

```cpp
		//set animation channel/cycle to be played
		for(Mesh* mesh : mesh_array)
			mesh->setSkinnedAnimationChannel("player_walking_cycle");
```
A `DirectionalLight` object is also instanced for lighting the scene:
```cpp
    //add directional light
    DirectionalLight* direct_light = new DirectionalLight;
```
All `Mesh` objects and `AnimationPlayer` must be updated in the game's while loop. This is done by calling the `update()` function for each class:

```cpp

while(!windowManager.shouldClose()){
    //update AnimationPlayer
    animation_player.update();
    
    //update/render all Mesh objects
		for(Mesh* mesh : mesh_array){
					
					//[optional] change render mode (triangles, point cloud, lines) if K or L pressed
					if( windowManager.isKkeyPressed() ) //[optional] render point cloud if K pressed
						mesh->setRenderingMode(GL_POINTS);
					else if( windowManager.isLkeyPressed() )//[optional] render lines if L pressed
						mesh->setRenderingMode(GL_LINES);
					else //default rendering mode (solid triangles), does not need to be specified if not using lines or points
						mesh->setRenderingMode(GL_TRIANGLES);
					
					
					//update
					mesh->update();
				}
    
}
```
A the position/rotation/scale of a mesh or `EmptyNode` can be set by calling `setPosition(glm::vec3)`, `setRotation(glm::quat)` and `setScale(glm::vec3)`:
```cpp

			//set EmptyNode to follow camera
			for(EmptyNode* empty : model->getRootNodesArray())
				//set pos
				empty->setTranslation( camera.GetPosition() - camera.GetUp()*30.f + camera.GetFront()*60.f );

			//set Mesh follow camera
			for(Mesh* mesh : mesh_array)
				mesh->setTranslation( camera.GetPosition() );

```

Skinning and model/mesh animations can be reset by calling `AnimationPlayer::resetAnimations` inside the while loop:
```cpp
    //reset animations if R pressed
    if(windowManager.isRKeyPressed())
        animation_player.resetAnimations();

```

Finally, once the program is ready to terminate, all of `Mesh`, `ModelLoader` and `DirectionalLight` instances which were allocated on the heap can be free'd (outside the while loop):

```cpp

int main(){

    while(!windowManager.shouldClose()){
        //update ..
    }
    
    //app terminating
    
		//delete Mesh* objects
		for(Mesh* mesh : mesh_array)
			delete mesh;
		
		//delete ModelLoader class
		delete model;
	
		//delete directional light
		delete direct_light;
}

```










## ModelLoader
========================
The `ModelLoader` class handles the process reading glTF files from disk, and extracting all mesh and animation data required for rendering and animation. It primarly interfaces with the `tinygltf` header for importing of glTF files. All extracted data is stored in separate arrays which can be accessed as public members of the class.

### ModelLoader::ModelLoader (constructor)
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
Data for each mesh is stored in `MeshDataStruct` object. A loop iterates of each `tinygltf::Mesh` object, and populates the data fields for each `MeshDataStruct`. Checks are first made to ensure the `tingltf::Mesh` object is not a bone or armature. Vertices, UV's, normals, indices, textures, materials, animations, static transforms and skinning data (joints/weights) is carried out for each mesh:
```cpp
//loop over all tinygltf::Mesh objects
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
		
		//VERTEX DATA
		mesh_data_struct->vertex_positions_array = getVertexPositions(mesh);
		
		//NORMAL DATA
		mesh_data_struct->vertex_normals_array = getVertexNormals(mesh);
		
		//UVS
		mesh_data_struct->vertex_uvs_array = getVertexUVs(mesh);
		
		//INDICES
		mesh_data_struct->vertex_indices_array = getIndices(mesh);
	
		//TEXTURES
		mesh_data_struct->texture_map = getTextureMap(mesh);
		
		
		//static pos/rot/scale
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
		
		//SKINNING [JOINTS/WEIGHTS]
		if(!model.skins.empty()){//need better checks than this
			
			has_skin = true;
			
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
		
		
		//ANIMATIONS
		mesh_data_struct->animation_data = getMeshAnimationData(mesh);
		
		
		//MATERIALS
		mesh_data_struct->material_data = getMaterial(mesh);
		
		//add to MeshDataStruct array
		mesh_data_struct_array.emplace_back(mesh_data_struct);
	}

```
Each `MeshDataStruct` gets allocated on the heap. After its fields are filled with data, a pointer to each object gets stored in the `ModelLoader::mesh_data_struct_array` array.


Creation and storage of animated `EmptyNode` objects (nodes which may hold animation and transform data) is carried out by iterating over the `tinygltf::Model::nodes` array. Checks are made to ensure this node is neither a mesh, nor a bone. Animations are loaded and stored in the `EmptyNode::animation_data` field, by calling [ModelLoader::getNodeAnimationData](ModelLoader.md#ModelLoader%3A%3AgetNodeAnimationData). Other data such as node static transforms are retrieved and stored by calling [ModelLoader::getTranslation](ModelLoader.md#ModelLoader%3A%3AgetTranslation), [ModelLoader::getRotation](ModelLoader.md#ModelLoader%3A%3AgetRotation), [ModelLoader::getScale](ModelLoader.md#ModelLoader%3A%3AgetScale), and [ModelLoader::getTransformMatrix](ModelLoader.md#ModelLoader%3A%3AgetTransformMatrix). Once all the data is filled in for each `EmptyNode` object, it gets added to the `ModelLoader::empties_array` array, which stores a pointer to the dynamically allocated `EmptyNode`. To avoid memory leaks, all `EmptyNode` are free'd in the `ModelLoader::~ModelLoader` destructor.
```cpp
//create, fill and store each EmptyNode objects
	for(int n{}; n<model.nodes.size(); n++){
	  //dynamically allocate EmptyNode object
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

		//check to see if has sub-nodes
		if(!node.children.empty()){
			empty->has_childs = true;
			empty->child_array = node.children;
		}
		
		
		//get animations for this node (if the exist)
		empty->animation_data = getNodeAnimationData(node);
		
		//check if this node is a root node
		empty->is_root = isRootNode(node);
		//fetch static transform data (pos/rot/scale/matrix)
		{
		//translation
			empty->translation = empty->animation_data.translation = getTranslation(node);
			
			//rotation
			empty->rotation = empty->animation_data.rotation = getRotation(node);
			
			//scale
			empty->scale = empty->animation_data.scale = getScale(node);
			
			//create TRS matrix (not needed if transform matrix is specified)
			empty->modelMatrix = createTRSmatrix(empty->translation, empty->rotation, empty->scale);//if it uses a custom `matrix_transform` in the glTF, then this will get overriden so don't worry 
			
			//transform matrix 
			if(!node.matrix.empty()){
				empty->matrix_transform = getTransformMatrix(node);
				empty->has_matrix_transform = true;
				empty->modelMatrix = empty->matrix_transform;
			}
		}
		
		//add to array
		empties_array.emplace_back(empty);
	}

```
Since data gaps might exist in the node animation, these must be filled in for smooth and valid playback. The [ModelLoader::fillInAnimationGaps](ModelLoader.md#ModelLoader%3A%3AfillInAnimationGaps) is called for each `EmptyNode`:
```cpp
	for(EmptyNode* empty : empties_array)
		fillInAnimationGaps( empty->animation_data );
```
The maximum timeline is calculated and updated for each `EmptyNode` object by calling [ModelLoader - ModelLoader::getMaxNodeTimeline](ModelLoader.md#ModelLoader%3A%3AgetMaxNodeTimeline) and storing this timeline in the `EmptyNode::animation_data::time_array` variable.

```cpp
	//get max Node timeline
	max_node_timeline = getMaxNodeTimeline();
	//update time arrays with max array for each EmptyNode
	for(EmptyNode* empty : empties_array)
		empty->animation_data.time_array = max_node_timeline;
```
Certain animation transforms may be shorter than others, depending on how the model is animated (e.g. translation durations could be shorter than rotation duration). All animation transform arrays must not be empty (greater than 0), and also of equal size for valid playback, hence [ModelLoader - ModelLoader::equalizeAndMatchNodeAnimations](ModelLoader.md#ModelLoader%3A%3AequalizeAndMatchNodeAnimations) is called next.

All skinning animations are called using [ModelLoader::getSkinnedAnimation](ModelLoader.md#ModelLoader%3A%3AgetSkinnedAnimation)

Since meshes can be parented to animated `EmptyNode`, it's important to establish an array which defines the linkage between them. Also, since certain `EmptyNode` objects may also be parented to other nodes, we must also take note of this as well:
```cpp

    //loop over all EmptyNode objects
	for(EmptyNode* empty : empties_array){
		
		//skip if bone
		if(isBone(empty->node_index))
			continue;
			
		//get parent node index (should not be -1 if has parent)
		int idx = getParentNodeIndex(empty->node);
		if(idx == -1)
			continue;
		
		std::pair<EmptyNode*, EmptyNode*> pair;
		
		//find root EmptyNode for this child
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
```
This hierarchy is stored in `ModelLoader::root_and_child_array`, where each entry is a `std::pair<EmptyNode*, EmptyNode*>` object. The first entry in each pair is the root `EmptyNode` (pointer), whilst the second is the child.

Since the code above doesn't not take into account `EmptyNode` objects which have no parent (i.e. start of the hierarchy tree), a special case must also be made for them. This time, a very simple check is made to see if the given node has a parent using [ModelLoader::getParentNodeIndex](ModelLoader.md#ModelLoader%3A%3AgetParentNodeIndex), and if not (-1), then it gets added to the `ModelLoader::root_array` array:
```cpp
    //loop over all EmptyNode objects
	for(EmptyNode* empty : empties_array){
		//skip if bone
		if(isBone(empty->node_index))
			continue;
		
		int idx = getParentNodeIndex(empty->node);
		if(idx == -1)//MUST be == -1 for it to be a root node
			root_array.emplace_back(empty);
	}
```


### ModelLoader::~ModelLoader (destructor)
Free'ing of all dynamically allocated memory for models, meshes, textures happens here.
All GL textures must be free'd using `glDeleteTextures`:
```cpp
	//delete all GL texture's once finished
	for(MeshDataStruct* mesh_data : mesh_data_struct_array){
		for(const auto& tex_data : mesh_data->texture_map){
			glDeleteTextures(1, &tex_data.second.tex_id);
		}
	}
```
All dynamically allocated `MeshDataStruct` and `EmptyNode` objects are also free'd using the `delete` keyword.


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

### ModelLoader::getTextureMap
Fetches all texture data for albedo, normal and specular maps. Raw image pixel data and other data for each image is stored inside the `tingltf::Image` struct. To access this struct, we need to find the texture index used inside `tinygltf::Texture`, and use its `tinygltf::Texture::source` variable as the index in the `tinygltf::Model::images` array. The `tinygltf::Texture` object can be retrieved by using a diffuse, normal or metalness index stored inside `tinygltf::Material`, and using that as a key in the `tinygltf::Model::textures`. For the diffuse texture:

```cpp
    tinygltf::Material mat = model.materials[mat_idx];

    //get diffuse texture material index 
    int diff_tex_idx = mat.pbrMetallicRoughness.baseColorTexture.index;

    //fetch tinygltf::Texture object
    tinygltf::Texture diffuse_textu = model.textures[diff_tex_idx];
    int diff_img_idx = diffuse_textu.source; 
    
    //finally get Image object (which stores texture data)
		tinygltf::Image diffuse_image = model.images[diff_img_idx];

```
Now we can generate OpenGL texture objects, which are used in the shader.

```cpp
    //create opengl texture object
		glGenTextures(1, &diffuse_texture);
		
		//bind to gl texture
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		//set mipmapping options (far and close)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //far away texture mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //
		
		//set wrapping options (x and y)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		//fill gl texture buffer with raw image data stored in tinygltf::Image
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diffuse_image.width, diffuse_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuse_image.image.data());
		
		//must be called for mipmaiing to work
		glGenerateMipmap(GL_TEXTURE_2D);
		
		//unbind once finished
		glBindTexture(GL_TEXTURE_2D, 0);

```
Same process is carried out to generate normal and metal textures. However, normal texture color format is specified as `GL_SRGB_ALPHA` and not `GL_RGBA`:
```cpp
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, normal_image.width, normal_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, normal_image.image.data());
		
```

### ModelLoader::getMaterial
The `tinygltf::Material` stores all material related data some certain texture index info. This include PBR data as well, such as base color, metalness, roughness and emissiveness.

### ModelLoader::fillInAnimationGaps
Sometimes, depending on how a model is being animated, there can be gaps between keyframes in an animation. This occurs when transform data (position, rotation and scale) are animated at discrete points in an animation timeline, causing undefined/non-existent data gaps when parsing the glTF file. To work around this, we must figure out what data must be present at these missing keyframes, and fill in all data gaps for translation, rotation and scale animation arrays. The missing data will always be the same as the last known data in the last animated keyframe (which is already known after parsing). The first step is to figure out how many missing keyframes there are, and at what indexed locations they occur in an animated timeline. This can easily be done by calculating the delta time between each two frames (future and present frames), and checking if they exceed the fixed time step of 0.03333 seconds (at 30 frames per second = 1/30). The number of missing frames can be calculated by dividing this delta time by the fixed time step. Taking the animated translations as an example:
```cpp
		std::map<int, int> gap_idx_trans_array;
		
		int summed_gap_size {};
		//iterate over translation animation data array (which assuming has gaps)
		for(int y{}; y<animation_data.translation_anim_array.size() - 1; y++){
		//calculate the delta time between the future frame y + 1) and present frame (y)
			float curr_delta = animation_data.trans_time_array[y+1]-animation_data.trans_time_array[y];

      //calculate gap size (number of missing elements) by dividing by DELTA_TIME_STEP (= 0.03333)
			int gap_steps = std::round(curr_delta/DELTA_TIME_STEP);
		}

```
Now we can fill in the missing data:
```cpp

		for(const auto& itr : gap_array){
			for(int i{}; i<itr.second; i++){
			//missing data will be equivalent to the last know animated data keyframe
				glm::vec3 last_pos = animation_data.translation_anim_array[itr.first];
				
				//fill gaps in array using .insert()
	animation_data.translation_anim_array.insert(animation_data.translation_anim_array.begin() + itr.first + 1, last_pos);
			}
		}
```
The same process applies to filling in data gaps for rotation and scale animation arrays.

### ModelLoader::equalizeAndMatchNodeAnimations
This function scales up all `EmptyNode` animation channels (position, rotation, scale) to match the maximum timeline array size. It also fills all empty (size 0) channels with static data (required for animation system to function).

### ModelLoader::getTimelineArray
Pulls raw keyframe time data for a given `tinygltf::AnimationSampler`. Works very similarly to what is described for [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions). I.e. the `tinygltf::Accessor` for the given sampler is retrieved from the `tonygltf::Model::accessors` array, based on the sampler input index `tinygltf::AnimationSampler::input`. Then `byteOffset` and `offset` can be found easily:
```cpp
    //get time data accessor
		tinygltf::Accessor time_accessor = model.accessors[input_idx];
		int frame_count = time_accessor.count;
		
		int byteOffset = model.bufferViews[time_accessor.bufferView].byteOffset;
		
		int offset = byteOffset/sizeof(float);

```
Raw time data is fetched from the `float_array` using the `offset` variable as the index. However, the current time must be subtracted from the starting time, to ensure the real time data (with no starting offsets) is used:
```cpp
     //loop over keyframes and get time data
		for(int t{}; t<frame_count; t++){
			float start_time = float_array[offset];
			float time_ = float_array[t + offset] - start_time; //subtract `start_time` to get it 0 initialized
		}
```

### ModelLoader::getMaxNodeTimeline
Creates the largest complete timeline for use with `EmptyNode` animations. First, the maximum duration for all `EmptyNode` animations (for translation, scale and rotation) is stored in an array, and the timeline with the largest value is selected. Since this timeline may contain data gaps, the same method described in [ModelLoader - ModelLoader::fillInAnimationGaps](ModelLoader.md#ModelLoader%3A%3AfillInAnimationGaps) is applied to fill in any missing floating point time data:
```cpp
		int summed_gap_size {};
		for(int y{}; y<max_timeline.size() - 1; y++){
			float curr_delta = max_timeline[y+1]-max_timeline[y];
			float delta_time = TIME_STEP;
			int gap_steps = std::round(curr_delta/delta_time);

			if(curr_delta >= delta_time*1.2f){
 
				gap_idx_trans_array.emplace(y + summed_gap_size, gap_steps - 1);
				
				summed_gap_size += gap_steps - 1; 
			}
		}
		
		//fill in missing times
		for(const auto& itr : gap_idx_trans_array){
			float delta = max_timeline[itr.first] + TIME_STEP;
			for(int i{}; i<itr.second; i++){
				max_timeline.insert(max_timeline.begin() + itr.first + 1 + i, delta);
				delta += TIME_STEP;
			}
		}

```
Once complete, there will be a contiguous maximum timeline which is retured for use in `EmptyNode` animation playback.

### ModelLoader::getNodeAnimationData
Models can typically be parented to blank nodes (`EmptyNode`), which have animations. Extracting all of translation, rotation and scale animation data (as well as thier timelines) is carried out in this function. Firstly, the `tinygltf::Animation` object is retrieved for the given node, based on its index. In order to fetch the raw animation data for all animated keyframes, we need to use the `output` variable (stored in `tinygltf::AnimationSampler`) as an index to retrieve the corresponding `tinygltf::Accessor` accessor for this node. The accessor will give us essential `byteOffset` and frame count data required to fetch the animation data from the `buffer` data block, similar to what is explained in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions).

```cpp

	for(int c{}; c<animation.channels.size(); c++){
			tinygltf::AnimationChannel& channel = animation.channels[c];
		tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];
	
	
	    int output_idx = sampler.output;
	    
	    //get accessor using output_idx
    		tinygltf::Accessor& accessor = model.accessors[output_idx];
    		
    		int frame_count = model.accessors[output_idx].count;
    		int byteOffset = model.bufferViews[accessor.bufferView].byteOffset;

       int offset = byteOffset/getSizeOfComponentType(accessor.componentType);
	
	}

```
Once we have `byteOffset` and `offset` integers, we can loop over the total frame count and fetch the raw translation, rotation and scale data. These are read from the cast `float_array`, in exactly the same way described in [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions). Taking animated translation as an example, we have:
```cpp

    //fetch animated translations
    for(int i{}; i<frame_count; i++){
				float x = float_array[(i*3) + 0 + offset];
				float y = float_array[(i*3) + 1 + offset];
				float z = float_array[(i*3) + 2 + offset];
				
				//translation for current frame
				glm::vec3 current_translation = glm::vec3(x, y, z);

         //fetch raw time data for translations
				animation_data.trans_time_array = getTimelineArray(sampler);
			}
				

```
The same method applies for fetching rotation data. However, since quaternions are used, a 4th component (`w`) must be retrieved:
```cpp
    //fetch animated rotations
    for(int i{}; i<frame_count; i++){
				float x = float_array[(i*4) + 0 + offset];
				float y = float_array[(i*4) + 1 + offset];
				float z = float_array[(i*4) + 2 + offset];
				float w = float_array[(i*4) + 3 + offset];
				
				//rotations for current frame
				glm::quat current_rotation = glm::quat(w, x, y, z);

         //fetch raw time data for rotations
				animation_data.rot_time_array = getTimelineArray(sampler);
			}

```
Animated scale data is also pulled in the same way described for translations.

### ModelLoader::getMeshAnimationData
Fetches any animations for a given `tinygltf::Mesh` object. The process is similar to what has been described in  [ModelLoader::getNodeAnimationData](ModelLoader.md#ModelLoader%3A%3AgetNodeAnimationData). If no animations exist for the given mesh, then the fucntion exits.


### ModelLoader::getTranslation
Gets the static translation for a given node, by querying the `tinygltf::Node::translation` variable (if specified for this node).
```cpp
	if(!node.translation.empty())
		glm::vec3 translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
```
### ModelLoader::getRotation
Gets the static rotation for a given node, by querying the `tinygltf::Node::rotation` variable (if specified for this node).
```cpp
	if(!node.rotation.empty())
		glm::quat rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
```
### ModelLoader::getScale
Gets the static scale for a given node, by querying the `tinygltf::Node::scale` variable (if specified for this node).
```cpp
	if(!node.scale.empty())
		glm::vec3 scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
```

### ModelLoader::getTransformMatrix
Gets the static transform matrix for a given node, by querying the `tinygltf::Node::matrix` variable (if specified for this node).
```cpp
	if(!node.matrix.empty()){
		glm::mat4 matrix = glm::mat4(
			node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
			node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
			node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
			node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]
			);
	}
```

### ModelLoader::getMaxSkinnedDuration
Gets the largest animation time value for a skinned mesh. This is achieved by storing all time values for all transform animations (translation, rotation, scale), per bone, and finding the largest value from these arrays. This maximum value will be used to construct a final timeline array for skinned animation playback.

### ModelLoader::getSkinnedAnimation
This function handles fetching animated transform data for each bone in a skinned mesh. Pulling this data is very similar to what has been described in [ModelLoader::getNodeAnimationData](ModelLoader.md#ModelLoader%3A%3AgetNodeAnimationData) and [`ModelLoader::getVertexPositions`](ModelLoader.md#ModelLoader%3A%3AgetVertexPositions). All bone node indices are stored in the  `tinygltf::Skin::joints` array.

```cpp
	tinygltf::Skin& skin = model.skins.front();
	
	std::vector<int> skin_node_indices = skin.joints;
```
It's important to note that a skinned mesh may have multiple discrete/separate animation cycles (or channels). For example, a character mesh could have run animation cycle, walk cycle, jump cycle, idle cycle, crouch cycle, etc. These cycles all have unique names, and their `tinygltf::Animation` objects retrieved and stored in a map:


```cpp
    //stores tinygltf::Animation object for each animtion cycle, with the key being its name (std::string)
	std::map<std::string, tinygltf::Animation> animation_gltf_map;
	
	//find animation channels/cycles for this skin
	for(const tinygltf::Animation& a : model.animations){
		for(const tinygltf::AnimationChannel& c : a.channels){
			if(std::find(skin_node_indices.begin(), skin_node_indices.end(), c.target_node) != skin_node_indices.end()){
				animation_gltf_map.emplace(a.name, a);
			}
		}
	}

```
We can now iterate over each cycle and start working on pulling its raw transform and time data. Getting time data is the same as what is described in [ModelLoader::getTimelineArray](ModelLoader.md#ModelLoader%3A%3AgetTimelineArray). Fetching the animated translation, rotation and scale data per bone is also excatly the same as decribed [ModelLoader::getNodeAnimationData](ModelLoader.md#ModelLoader%3A%3AgetNodeAnimationData). The only new code block is one which checks if a bone has a root bone (required for calculating final transform matrix pose of the child bone):
```cpp
  
  //iterate over each animation cycle/channel
  std::map<std::string, tinygltf::Animation> animation_gltf_map;
	for(const auto& anim_map_itr : animation_gltf_map){
  
    	tinygltf::Animation anim = anim_map_itr.second;
    	
    	//stores timeline array for EACH bone
    	std::map<int, std::vector<float>> timeline_map;
    	
    	//stores anim data for EACH bone
    	std::map<int, AnimationDataStruct> bone_anim_map;
    	
    	//fetch time data (same process in ModelLoader::getTimelineArray)
    	for(int c{}; c<anim.channels.size(); c++){
    	    //.... get time data
    	}
    
		
		//fetch bone animated transform data (same process in ModelLoader::getNodeAnimationData)
		for(int c{}; c<anim.channels.size(); c++){
		
		//find root bone
		for(std::size_t n{}; n<model.nodes.size(); n++){
			std::vector<int> childs_vec = model.nodes[n].children;
			if( std::find(childs_vec.begin(), childs_vec.end(), node_idx) != childs_vec.end() ){
				animation_data.has_root = true;
				animation_data.root_idx = n;
				break;
			}
		}
		
		//.... get translation/rotation/scale data per bone
		}

    }

```
The maximum timeline (without gaps) can be calculated, by calling [ModelLoader::getMaxSkinnedDuration](ModelLoader.md#ModelLoader%3A%3AgetMaxSkinnedDuration) to get the maximum end time, and filling the timeline array incrementally in steps of `TIME_STEP` (0.03333):
```cpp
    // time_array is the maximum timeline with no gaps
		std::vector<float> time_array;
		float tt {};//current time
		float max_time = getMaxSkinnedDuration(bone_anim_map);
		while(tt<max_time){
			time_array.emplace_back(tt);
			tt+=TIME_STEP;
		}

```
Since data gaps might exist, these must be filled in for smooth and valid playback. The [ModelLoader::fillInAnimationGaps](ModelLoader.md#ModelLoader%3A%3AfillInAnimationGaps) is called for each bone:
```cpp

		for(auto& v : bone_anim_map){

			AnimationDataStruct& animation_data = v.second;

			fillInAnimationGaps(animation_data);
		}
```
Due to a combination of how a model is animated and certain glTF exporters, some animation timelines may contain animation data for more than one playback rate (e.g. 24 FPS and 30 FPS). Only one playback speed is needed, while the rest can be discarded. To figure out which extra frames must be discarded/erased from the timelines, we can simply divide each keyframe time data by a fixed delta time value `TIME_STEP` (which assumes we will use 1/`TIME_STEP` as the desired FPS playback rate), and if the absolute value of the remainer is larger than 0, then it must be an unneeded frame. Taking translation animations as an example:
```cpp
    //loop over each bone
		for(auto& v : bone_anim_map){
			AnimationDataStruct& animation_data = v.second;

			//TRANSLATION DATA
			
			int removed_cnt = 0;
			std::vector<int> gap_idx_map;
			for(int i{}; i<animation_data.trans_time_array.size(); i++){
			    //check if absolute value of remainder is greater than 0 (0.0001f due to floating point/conversion errors)
				if( std::abs( std::remainderf(animation_data.trans_time_array[i], TIME_STEP) ) > 0.00001f ){

					gap_idx_map.emplace_back(i - removed_cnt);

					removed_cnt += 1;
				}
			}
			
			//erase the extra frames in both time and translation animation arrays
			for(int i : gap_idx_map){
				animation_data.trans_time_array.erase(animation_data.trans_time_array.begin() + i);
				animation_data.translation_anim_array.erase(animation_data.translation_anim_array.begin() + i);
			}
		}
			
```
The same method is applied to clean up rotation and scale arrays from extra data which will not be used.
Static transform data (translation/rotation/scale) for each bone is retrieved for each bone in the same way mentioned in [ModelLoader::getTranslation](ModelLoader.md#ModelLoader%3A%3AgetTranslation), [ModelLoader::getRotation](ModelLoader.md#ModelLoader%3A%3AgetRotation), [ModelLoader::getScale](ModelLoader.md#ModelLoader%3A%3AgetScale).
Since the animation system requires all arrays to be non-empty, all empty arrays (size 0) or arrays with size 1 are filled with static values to ensure animation validity. Taking translation as an example:
```cpp
		//equalize empty arrays
		//iterate over each bone
		for(auto& v : bone_anim_map){
			AnimationDataStruct& animation_data = v.second;
			
			//if size == 0
			//translation
			if(animation_data.translation_anim_array.empty())
				for(float t : time_array)	animation_data.translation_anim_array.emplace_back(animation_data.translation);
				    animation_data.translation_anim_array.emplace_back(animation_data.translation);
			//if size == 1
			if(animation_data.translation_anim_array.size() == 1)
				for(int y{}; y<time_array.size() - 1; y++)
    				animation_data.translation_anim_array.emplace_back(animation_data.translation_anim_array.front());
			
			//same process above is applied for rotation and scale animation arrays

		}

```
Since all animation arrays must be the same size for the animation system to function (otherwise desync issues occur), all of translation, rotation and scale arrays are scaled up to match the maximum size of the largest time array. Taking translation animation arrays as an example:

```cpp
    //loop over each bone
		for(auto& itr : bone_anim_map){
			AnimationDataStruct& animation_data = itr.second;
			
			int max_size = time_array.size();
			
			//equalize translation array
			if(animation_data.translation_anim_array.size() != max_size){
				//if few frames short of max_size, calculate difference and fill in
				int diff = max_size - animation_data.translation_anim_array.size();
				glm::vec3 last_pos = animation_data.translation_anim_array.back();
				for(int i{}; i<diff; i++)
					animation_data.translation_anim_array.emplace_back( last_pos );
			}
			
			
			//same applies for rotation and scale
		}

```
Finally, all animations get added to `ModelLoader::bone_animation_channel_map` map, with the key being the name of the animation channel/cycle for easy access.

### ModelLoader::getParentNodeIndex
Checks if a given `tinygltf::Node` has a parent. If so, returns the index of that parent node, which can directly be used to retrieve the node from `tinygltf::Model::nodes` array. If the index is -1, then the node has no parent.








































## AnimationPlayer
========================
The `AnimationPlayer` is responsible for updating all animation of `EmptyNode` objects in realtime, as well as updating the transforms of any linked meshes. Since animating and updating the transforms of linked nodes and meshes must respect a strict inheritance hierarchy, the transforms are calculated starting from the root/base node, and follow the path to the final mesh.

### AnimationPlayer::AnimationPlayer (constructor)
Currently does nothing other than initializing member variables.

### AnimationPlayer::update
Since the order of node animation playback must follow the order of the inheritance tree, the first nodes to have their animations updated must be the root/start nodes in the tree. A loop iterates over all root `EmptyNode` and updates and applies transforms:
```cpp
    //loop over all root nodes
	for(EmptyNode* root_empty : model_loader->getRootNodesArray()){
		AnimationDataStruct& animation_data = root_empty->animation_data;
		
		glm::mat4 final_matrix = glm::mat4(1.f);
    
    //update time
		animation_data.current_animation_time += window_manager->GetDeltaTime() * animation_data.playback_speed;
		
		//UPDATE EMPTY ANIMATIONS POS/ROT/SCALE [if they exist]
		glm::vec3 anim_position = calculateCurrentTranslation(animation_data);
		glm::quat anim_rotation = calculateCurrentRotation(animation_data);
		glm::vec3 anim_scale = calculateCurrentScale(animation_data);
		
		//animated TRS matrix [if animated]
		glm::mat4 anim_matrix = createTRSmatrix(anim_position, anim_rotation, anim_scale);
		
		root_empty->modelMatrix = createTRSmatrix(root_empty->translation, root_empty->rotation, root_empty->scale) * anim_matrix * root_empty->matrix_transform;//if this node is animated, its final matrix will be = root matrix * animted TRS matrix
	}

```
The [AnimationPlayer - AnimationPlayer::calculateCurrentTranslation](AnimationPlayer.md#AnimationPlayer%3A%3AcalculateCurrentTranslation), [AnimationPlayer - AnimationPlayer::calculateCurrentRotation](AnimationPlayer.md#AnimationPlayer%3A%3AcalculateCurrentRotation) and [AnimationPlayer - AnimationPlayer::calculateCurrentScale](AnimationPlayer.md#AnimationPlayer%3A%3AcalculateCurrentScale) are called to update the animated transforms (if they exist). The final transform is obtained by multiplying the animated transform and static transforms together.

Next, all subnodes/child `EmptyNode` objects are updated. The process follows what was mentioned above for root nodes. However, this time any child mesh nodes are updated with the current animated transforms via the `MeshDataStruct` object.

```cpp
	for(const auto& pair_itr : model_loader->getRootAndChildArray()){
	    //root (parent)
		EmptyNode* empty_root = pair_itr.first; 
		//child node
		EmptyNode* empty_child = pair_itr.second;
		
		//animation data struct for child
		AnimationDataStruct& anim_data_child = empty_child->animation_data;
		
		glm::mat4 root_matrix = empty_root->modelMatrix;
		
		anim_data_child.current_animation_time += window_manager->GetDeltaTime() * anim_data_child.playback_speed;
		
		//UPDATE EMPTY ANIMATIONS POS/ROT/SCALE [if they exist]
		glm::vec3 child_anim_position = calculateCurrentTranslation(anim_data_child);
		glm::quat child_anim_rotation = calculateCurrentRotation(anim_data_child);
		glm::vec3 child_anim_scale = calculateCurrentScale(anim_data_child);
		
		//animated TRS matrix [if animated]
		glm::mat4 child_anim_matrix = createTRSmatrix(child_anim_position, child_anim_rotation, child_anim_scale);
		
		if(empty_child->animation_data.has_animation){
			empty_child->modelMatrix = root_matrix * child_anim_matrix;//if this node is animated, its final matrix will be = root matrix * animted TRS matrix
		}
		else{
			//if uses matrix instead of raw pos/rot/scale data supplied from gltf
			if(empty_child->has_matrix_transform)
				empty_child->modelMatrix = root_matrix * empty_child->matrix_transform;
			else
				empty_child->modelMatrix = root_matrix * createTRSmatrix(empty_child->translation, empty_child->rotation, empty_child->scale);//if this node is NOT animated, its final matrix will be = root matrix * static TRS matrix
		}
		
		//get child meshes (provided they exist) for this EmptyNode
		std::vector<MeshDataStruct*> child_mesh_array = model_loader->getChildMeshArray(model_tinygltf.nodes[empty_child->node_index]);
		
		//update child mesh transforms
		if(!child_mesh_array.empty()){
			for(MeshDataStruct* msh_data : child_mesh_array){
				if(msh_data->node_index == -1)
					continue;
				
				//update transform matrix
				glm::mat4 mesh_model_matrix = createTRSmatrix(msh_data->translation, msh_data->rotation, msh_data->scale);//model_loader matrix of static pos/rot/scale
				mesh_index_map[msh_data->node_index]->mesh_data->modelMatrix = mesh_model_matrix * empty_child->modelMatrix;
				mesh_index_map[msh_data->node_index]->mesh_data->inherits_animation = true;
				
			}
		}
	}
	

```
The final transformed matrix of the mesh will there be the product of its static TRS matrix with the transform matrix of its parent `EmptyNode`.

### AnimationPlayer::calculateCurrentTranslation
Calculates the new translation for the current time value (`AnimationDataStruct::current_animation_time`). This is calculated by linearly interpolationg (lerping or blending) between the translation value for the current frame (i) and translation value for the future frame (i + 1). Interpolation is crucial, as it allows smooth and consistent playback of animations regardless of frame rate, and ensures animation data is present for every tick of the application's update loop:
```cpp
  //loop over time array
	for (int i{}; i < time_array.size(); i++) {
		float new_t = time_array[i + 1];
		float old_t = time_array[i];
		
		if (animation_data.current_animation_time <= new_t) {
		  //blend current frame translation with future translation (for next/upcoming frame) 
			float lerp = 1.f - (new_t - animation_data.current_animation_time) / (new_t - old_t);
			
			glm::vec3 old_pos = glm::vec3(trans_array[i].x, trans_array[i].y, trans_array[i].z);
			glm::vec3 new_pos = glm::vec3(trans_array[i + 1].x, trans_array[i + 1].y, trans_array[i + 1].z);
			
			//blending/lerping function
			final_mesh_pos = glm::mix(old_pos, new_pos, lerp);
			
			break;
		}
```
The result of the interpolated translation value is returned for the current frame.

### AnimationPlayer::calculateCurrentRotation
Calculates the new rotation for the current time value (`AnimationDataStruct::current_animation_time`). The same procedure described in [AnimationPlayer - AnimationPlayer::calculateCurrentTranslation](AnimationPlayer.md#AnimationPlayer%3A%3AcalculateCurrentTranslation) is applied. However, spherical linear interpolation (slerp) is carried out between any two rotations, and not linear interpolation (unless an specific edge case is met, which is desscribed below). Also, extra edge-case checks must be made for rotation quaternions. Firstly, the dot product between the current rotation (i) and future rotation (i + 1) is computed, and if this value is negative, then the inverse of `new_rot` (future rotation) is used. This is required since we always want quaternion spherical linear interpolation (slerp) to take the shortest path (otherwise it will yield incorrect animations). 

```cpp

			//EDGE CASE
			//TAKE SHORTEST PATH (if dot product yields negative)
			float quat_dot = glm::dot(old_rot, new_rot);
			if( quat_dot < 0.f ){
		    //inverse of new_rot is used
				new_rot = -new_rot;
			}
```

Another edge case that may occur is when the current rotation (`old_rot`) and future rotation (`new_rot`) are very close to each other. The dot product in that case will be close to 1.0. When two quaternions are very close to each other, we can use normal linear interpolation instead and not spherical linear interpolation:

```cpp
			//EDGE CASE
			//if dot product is almost 1.f, can lerp between vec4
			if( quat_dot > 0.993f ){
			    //convert current rotation to vec4
				glm::vec4 old_r = glm::vec4(old_rot.x, old_rot.y, old_rot.z, old_rot.w);
				
				//convert future rotation to vec4
				glm::vec4 new_r = glm::vec4(new_rot.x, new_rot.y, new_rot.z, new_rot.w);
				
				//linearly interpolate
				glm::vec4 lerped_r = glm::mix(old_r, new_r, lerp);
				
			}

```

### AnimationPlayer::calculateCurrentScale
Calculates the new scale for the current time value (`AnimationDataStruct::current_animation_time`). The same procedure described in [AnimationPlayer - AnimationPlayer::calculateCurrentTranslation](AnimationPlayer.md#AnimationPlayer%3A%3AcalculateCurrentTranslation) is applied.



















## VAO
========================
The Vertex Array Object (VAO) holds VBO data required for rendering a mesh. A more detailed description can be found here https://ogldev.org/www/tutorial32/tutorial32.html

### VAO::VAO (constructor)
When the constructor is called, a call to `glGenVertexArrays(1, &vao)` is made. The GL index of the VAO object is stored in the `VAO::vao` variable

### VAO::free
This free's the memory of the VAO object, which has been allocated by the GL driver (to avoid memory leaks once finished). The function `glDeleteVertexArrays(1, &vao)` carries this out.

### VAO::bind
This function calls `glBindVertexArray(vao)`, which binds the VAO to be the current active one used for rendering.

### VAO::unbind
Unbinding the currently active VAO is important once rendering a given mesh is finished, since it avoid undesirably modifying the VAO. Unbinding is achieved by calling `glBindVertexArray(0)` and passing 0 as the argument.VBO
#


# VBO
========================
The Vertex Buffer Object (VBO) holds the raw vertex data (coordinates, UV's, etc). A more detailed description can be found here https://opentk.net/learn/chapter1/2-hello-triangle.html#vertex-array-object

### VBO::VBO (constructor)
Generates a VBO object for use by the GL backend. Raw vertex data is passed in as the argument. When the constructor is called, a new VBO is generated via the GL function call `glGenBuffers(1, &vbo)`. The handle to the VBO object is a GLuint `VBO::vbo`. To pass the raw vertex data to the VBO buffer, we first bind the `VBO::vbo`, and then call `glBufferData`:
```cpp
//generate GL vbo
  glGenBuffers(1, &vbo);
  
  //bind
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  
  //fill buffer with raw vertex data
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*tri_verts.size(), tri_verts.data(), GL_STATIC_DRAW);//fill VBO with vert data

```

### VBO::free
This free's the memory of the VBO object, which has been allocated by the GL driver (to avoid memory leaks once finished). The function `glDeleteBuffers(1, &vbo)` carries this out.

### VBO::bind
This function calls `glBindBuffer(GL_ARRAY_BUFFER, vbo)`, which binds the VBO to be the current active one used for rendering.

### VBO::unbind
Unbinding the currently active VBO is important once rendering a given mesh is finished, since it avoid undesirably modifying the VBO. Unbinding is achieved by calling `glBindBuffer(GL_ARRAY_BUFFER, 0)` and passing 0 as the second argument.
## EBO
========================
The Element Buffer Object (EBO) holds shared indexed data used for rendering a triangle, without wasting extra index memory. Having an EBO allows for more optimized rendering, since it uses shared indices to render any adjacent triangles, without having to duplicate the indices (thereby saving on VRAM). A more detailed explanation can be found here: https://opentk.net/learn/chapter1/3-element-buffer-objects.html

### EBO::EBO (constructor)
Generates a EBO object for use by the GL backend. Raw index data is passed in as the argument. When the constructor is called, a new EBO is generated via the GL function call `glGenBuffers(1, &ebo)`. The handle to the EBO object is a GLuint `EBO::ebo`. To pass the raw vertex data to the EBO buffer, we first bind the `EBO::ebo`, and then call `glBufferData`:
```cpp
//generate GL ebo
  glGenBuffers(1, &ebo);
  
  //bind to current ebo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  
  //fill ebo buffer with raw index data
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*tri_indices.size(), tri_indices.data(), GL_STATIC_DRAW);

```

### EBO::free
This free's the memory of the EBO object, which has been allocated by the GL driver (to avoid memory leaks once finished). The function `glDeleteBuffers(1, &ebo)` carries this out.

### EBO::bind
This function calls `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo)`, which binds the EBO to be the current active one used for rendering.

### EBO::unbind
Unbinding the currently active EBO is important once rendering a given mesh is finished, since it avoid undesirably modifying the EBO. Unbinding is achieved by calling `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)` and passing 0 as the second argument.
## Mesh
========================
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













#

# default_shader
========================
The `default_shader.frag` and `default_shader.vert` are the respective fragment and vertex shaders used for rendering the model. This includes updating skinning and lighting.


### default_shader.frag (fragment shader)
Carries out the lighting and applies texturing and shading to the mesh. The bulk of shading is computed in [default_shader.frag - directionalLightFunc()](default_shader.md#directionalLightFunc%28%29)

#### directionalLightFunc()
Phong lighting model is used for shading with a single directional light. Details of phong shading can be found here: https://riptutorial.com/opengl/example/14741/phong-lighting-model. Since phong shading relies on a specular map, a basic specular map is quickly calculated by averaging out the colors of the metal map:

```glsl
  vec4 specularColor = vec4((texture(metal_tex, uvCoord).r + texture(metal_tex, uvCoord).g + texture(metal_tex, uvCoord).b)/3.f);
```
The code that follows does a series of calculations to obtain various view and light directional info, which is used in the phong equations:

```glsl
    //normal map
  vec3 normal = normal_mapping();
  
  //normalize light direction
  vec3 light_dir = -normalize(light_dir);
  
  //view direction (from perspective of each vertex)
  vec3 view_dir = normalize(vertCoord - cameraPos);
  
  //get light reflection vec
  vec3 light_reflect = reflect(light_dir, normal);

    //phong diffuse calc
  float diffuse_part = clamp(dot(light_dir, normal), 0.f, 1.f) * light_strength + AMBIENT_INTENSITY ;

  //phong specular calc 
  float spec_part = pow(clamp(dot(view_dir, light_reflect), 0.f, 1.f), SPECULAR_BEAD_SHARPNESS) * light_strength * light_specular;

  //apply phong shading
  vec4 final_color = (diffuseColor*diffuse_part) + (specularColor*spec_part) ;

	return final_color * vec4(light_color, 1.f);

```
#### normal_mapping()
This function returns a normal map for each fragment of a sampled normal map texture.
Details of equations and derivations can be found here: https://ogldev.org/www/tutorial26/tutorial26.html. In summary, a normal vector is converted from normal map color space to 3D space:

```glsl
//convert normal fragment color to 3D vector
vec3 normal_map = normalize( texture2D(normal_tex, uvCoord).rgb * 2.f - vec3(1.f) );

```
A tangent-bitangent-normal (TBN) 3x3 matrix is then created, which is then multipled by the converted normal in 3D space to get the final normal vector for use in the phong lighting calculations:
```glsl

//create tbn matrix
  mat3 tbn = mat3(tangentVec, bitangentVec, perpNormalVec);

//final normal
  vec3 normal = normalize(tbn * normal_map);

```

#### main()
The final shaded fragment color ouput is the product of the result of [default_shader - directionalLightFunc()](default_shader.md#directionalLightFunc%28%29) with a `base_color` value (can be set to plain white `vec3(1.f)`):

```glsl

fragColor = directionalLightFunc() * vec4(base_color.xyz, 0.f);
```

### default_shader.vert (vertex shader)
Deforming vertices with skin matrices, as well as finalizing vertex transforms are computed in this function. Tangent, bitangent and perpendicular normal vectors are also partially computed and passed to the fragment shader (along with UV data) as well.

#### main()
Assuming the mesh is skinned, the first step is to calculate the skinning matrix. This will be used directly to deform each vertex based on the closest bone poses. The skinning matrix is calculated by multiplying each component of the `weights_0` vector, by each component of the `inverseBindMatrix`. Each `inverseBindMatrix` is obtained from the `inverseBindMatrixArray` array using `joints_0` vector's indices as a key:

```glsl

      mat4 skinnedMatrix =
                        ( weights_0.x * inverseBindMatrixArray[int(joints_0.x)].matrix ) +
                        ( weights_0.y * inverseBindMatrixArray[int(joints_0.y)].matrix ) +
                        ( weights_0.z * inverseBindMatrixArray[int(joints_0.z)].matrix ) +
                        ( weights_0.w * inverseBindMatrixArray[int(joints_0.w)].matrix );
```
The final vertex positions output from the shader will be the product of the projection, view, model, skinning matrices, and the input vertex coordinates into the shader (raw vertex coordinate data):
```glsl
gl_Position = projMatrix * viewMatrix * modelMatrix * skinnedMatrix * vec4(vertPos.xyz, 1.f);
```

However, if the mesh is not skinned, then we can skip the skinning matrix loop and simply output the vertex positions by multiplying the projection, view, model matrices, and input vertex coordinates:
```glsl

gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertPos.xyz, 1.f);
```

Since the fragment shader needs the UV's, vertex coordinates, vertex normals, tangent, bitangent and perpendicular normal vectors for lighting and normal mapping, it is more performant to partially calculate some of these vectors in the vertex shader and send them to the fragment shader (since vertex shaders run for every vertex and not every fragment, which means much less cycles and more optimized):

```glsl
//output uv's
  uvCoord = uvVertCoord;
  
  //output vertex coords
  vertCoord = vec3( (modelMatrix * vec4(vertPos.x, vertPos.y, vertPos.z, 1.f)).xyz );//transform local -> global coord
  
  //output normals
  vertNormal = vNormal;
  
  //output tangents
  tangentVec = normalize(vec3( (modelMatrix * vec4(vTangent.xyz, 0.f)).xyz ));
  
  //outpu bitangents
  bitangentVec = normalize(vec3( (modelMatrix * vec4(vBiTangent.xyz, 0.f)).xyz ));
  
  //output perp normals
  perpNormalVec = normalize(cross(bitangentVec, tangentVec));
```












## Utils
========================
The `Utils.h` file houses commonly used functions and structs.

### AnimationDataStruct
Stores all animation related data for a given animated object. This includes, but not limited to, animation name, time array, transform arrays, static transforms and other data

### MaterialDataStruct
Stores material related infor for an object. Its fields include material name, base color, metalness, roughness.

### TextureDataStruct
Stores the GL texture index (which is then passed to the fragment shader) and `TextureType` (diffuse/normal/metal/specular/roughness)

### MeshDataStruct
Stores all vertex, texture, material and animation data for a given mesh. Contains arrays which hold vertex positions, UV's, normals, indices. Static model data is also stored as well.

### EmptyNode
Stores animation, transform and hierarchy data for blank animated nodes. All animation data is used directly in `AnimationPlayer::update()`.

### DirectionalLight
Stores light strength, specular intensity, direction and color for a directional light.

### createTRSmatrix
Creates a transform matrix using supplied values of translation, rotation and scale.

### __GL_ERROR_THROW__
Checks for any GL related errors using `glGetError()`, and throws a `std::logic_error` if any issues occur.




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
