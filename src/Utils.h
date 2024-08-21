#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>

#include <glad/glad.h>

//basic print
inline void PRINT(const std::string& message){
	std::cout << message << std::endl;
}

//warning
inline void PRINT_WARN(const std::string& message){
	PRINT("\x1B[38;2;255;120;10m [Warn] - \x1B[0m" + message);
}

//print custom color
inline void PRINT_COLOR(const std::string& message, short red, short green, short blue){
	PRINT( std::string("\x1B[38;2;") + std::to_string(red) + ";" + std::to_string(green) + ";" + std::to_string(blue) + "m [Info] - \x1B[0m" + message);
}


inline void printGlmVec3(const glm::vec3& v){
	std::cout << "[x: " << v.x << ", y: " << v.y << ", z: " << v.z << "]" << std::endl;
}

inline void printGlmMat4(const glm::mat4& m){
	/*
	*/
	std::cout << "Mat4: " << std::endl;
	std::cout << "[x: " << m[0].x << ", y: " << m[0].y << ", z: " << m[0].z << ", w:" << m[0].w << "]" << std::endl;
	std::cout << "[x: " << m[1].x << ", y: " << m[1].y << ", z: " << m[1].z << ", w:" << m[1].w << "]" << std::endl;
	std::cout << "[x: " << m[2].x << ", y: " << m[2].y << ", z: " << m[2].z << ", w:" << m[2].w << "]" << std::endl;
	std::cout << "[x: " << m[3].x << ", y: " << m[3].y << ", z: " << m[3].z << ", w:" << m[3].w << "]" << std::endl;
	
}

inline void printGlmQuat(const glm::quat& q){
	/*
	*/
	std::cout << "[x: " << q.x << ", y: " << q.y << ", z: " << q.z << ", w:" << q.w << "]" << std::endl;
	
}

inline glm::mat4 createTRSmatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale){
	glm::mat4 mat(1.f);
	
	//apply translation
	mat = glm::translate(glm::mat4(1.f), position);
	
	//apply rotation
	glm::mat4 rotate_mat = glm::mat4(rotation);
	
	mat = glm::translate(glm::mat4(1.f), position) * glm::mat4(rotation);
	
	//finally apply scale
	mat = glm::scale(mat, scale);
	
	return mat;
}



#endif //UTILS_H
