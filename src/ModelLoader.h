#ifndef MODEL_LOADER_H //header include guard
#define MODEL_LOADER_H


#include "../external/tinygltf/tiny_gltf.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>

#include <glad/glad.h>

#include "ErrorLogger.h"
#include "Utils.h"

class ModelLoader {
public:
    explicit ModelLoader(const std::string& path);
    ~ModelLoader();

//    void Render();
		
		//vertex/index/normal/uv data
		std::vector<glm::vec3> getVertexPositions(const tinygltf::Mesh& mesh);
		std::vector<glm::vec3> getVertexNormals(const tinygltf::Mesh& mesh);
		std::vector<glm::vec2> getVertexUVs(const tinygltf::Mesh& mesh);
		std::vector<unsigned int> getIndices(const tinygltf::Mesh& mesh);
	
		std::vector<MeshDataStruct>& getMeshDataArray() {	return mesh_data_struct_array;	}
		std::vector<Empty>& getEmptiesArray() {	return empties_array;	}
	
		//get global translation/pos/rot
		glm::vec3 getTranslation(const tinygltf::Node& node) const; 
		glm::quat getRotation(const tinygltf::Node& node) const; 
		glm::vec3 getScale(const tinygltf::Node& node) const; 
		glm::mat4 getTransformMatrix(const tinygltf::Node& node) const; 
	
		//textures
		std::map<TextureType, TextureDataStruct> getTextureMap(const tinygltf::Mesh& mesh);
	
		//materials
		MaterialDataStruct getMaterial(const tinygltf::Mesh& mesh);
		
		//animations
		bool has_animation = false;
		std::string animation_name;
		std::vector<AnimationDataStruct> bone_animation_array;//contains list of all animations for this model, with key being the animation name
		AnimationDataStruct getMeshAnimationData(const tinygltf::Mesh& mesh);
		AnimationDataStruct getNodeAnimationData(const tinygltf::Node& node);
		void getAllNodeAnimationTimelines();
		std::map<int, std::vector<float>> node_timelines_map;
		std::vector<float> getMaxNodeTimeline();
		std::vector<float> getMaxSkinnedTimeline(const std::map<int, AnimationDataStruct>& bone_anim_map);
		void equalizeAndMatchNodeAnimations();
		std::vector<float> max_node_timeline;//stores the largest time array
		
		void getSkinnedAnimation();
		void equalizeTRSanimationArrays(AnimationDataStruct& animation_data);	
		std::vector<float> getTimelineArray(const tinygltf::AnimationSampler& sampler);
		void fillInAnimationGaps(AnimationDataStruct& animation_data);
		float getAveragedAnimationFps(const std::map<int, AnimationDataStruct>& bone_animation_map);
	
		//skinning
		bool has_skin = false;
		std::vector<glm::vec4> getSkinJoints(const tinygltf::Mesh& mesh);
		std::vector<glm::vec4> getSkinWeights(const tinygltf::Mesh& mesh);
		std::vector<glm::mat4> getInverseBindMatrices(const tinygltf::Mesh& mesh);
		bool isBone(int node_index);
		bool isArmature(int node_index);

		/* helper functions */
		long unsigned int getSizeOfComponentType(int component_type);
		int getMeshNodeIndex(const tinygltf::Mesh& mesh);
		tinygltf::Model& getTinyGltfModel() {	return model;	}
		void getHierarchy(const tinygltf::Node& node_in);
		int getNodeIndex(const tinygltf::Node& node);
		bool isRootNode(const tinygltf::Node& node);
		int getParentNodeIndex(const tinygltf::Node& node);
		std::vector<MeshDataStruct> getChildMeshArray(const tinygltf::Node& node);
	
	
private:
    tinygltf::TinyGLTF tiny_gltf;
    tinygltf::Model model;
		std::string mesh_name;
	
		//the raw data cast into float/int arrays
		float* float_array = nullptr;
		unsigned short* ushort_array = nullptr;
		unsigned int* uint_array = nullptr;
		unsigned char* uchar_array = nullptr;
	
		GLuint diffuse_texture {};
		GLuint normal_texture {};
		GLuint metal_texture {};
	
		std::vector<Empty> empties_array;
		std::vector<MeshDataStruct> mesh_data_struct_array;
	
		float DELTA_TIME_STEP = 0.f;
	
};



inline glm::vec3 compute_tangent(const glm::vec3& v0, const glm::vec2& uv0, const glm::vec3& v1, const glm::vec2& uv1, const glm::vec3& v2, const glm::vec2& uv2){
	glm::vec3 edge0 = v1 - v0;
	
	glm::vec3 edge1 = v2 - v0;
	
	glm::vec2 uv_0 = uv1 - uv0;
	glm::vec2 uv_1 = uv2 - uv0;
	
	float div = 1.f/(uv_0.x * uv_1.y - uv_1.x * uv_0.y);
	
	glm::vec3 tangent = glm::vec3(0.f);
	tangent.x = div * (uv_1.y*edge0.x - uv_0.y*edge1.x);
	tangent.y = div * (uv_1.y*edge0.y - uv_0.y*edge1.y);
	tangent.z = div * (uv_1.y*edge0.z - uv_0.y*edge1.z);
	
	glm::vec3 t = glm::normalize( tangent );
	
	return t;
}

inline glm::vec3 compute_bitangent(const glm::vec3& v0, const glm::vec2& uv0, const glm::vec3& v1, const glm::vec2& uv1, const glm::vec3& v2, const glm::vec2& uv2){
	glm::vec3 edge0 = v1 - v0;
	
	glm::vec3 edge1 = v2 - v0;
	
	glm::vec2 uv_0 = uv1 - uv0;
	glm::vec2 uv_1 = uv2 - uv0;
	
	float div = 1.f/(uv_0.x * uv_1.y - uv_1.x * uv_0.y);
	
	glm::vec3 bitangent = glm::vec3(0.f);
	bitangent.x = div * (-uv1.x*edge0.x + uv0.x*edge1.x);
	bitangent.y = div * (-uv1.x*edge0.y + uv0.x*edge1.y);
	bitangent.z = div * (-uv1.x*edge0.z + uv0.x*edge1.z);
	
	glm::vec3 bt = glm::normalize(bitangent);
	
	return bt;
}


#endif //MODEL_LOADER_H
