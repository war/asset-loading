#ifndef ERROR_LOGGER_H
#define ERROR_LOGGER_H

#include <stdexcept>
#include <string>

#include <glad/glad.h>

inline void __GL_ERROR_THROW__(const std::string& optional_string = ""){
	GLenum error_check = glGetError();
	
	if(error_check != GL_NO_ERROR){
		std::string core_message = "**** [ERROR] -- ";
		
		if(error_check == GL_INVALID_ENUM)
			core_message += optional_string + " (code: GL_INVALID_ENUM)";
		
		if(error_check == GL_INVALID_VALUE)
			core_message += optional_string + " (code: GL_INVALID_VALUE)";
		
		if(error_check == GL_INVALID_OPERATION)
			core_message += optional_string + " (code: GL_INVALID_OPERATION)";
		
		if(error_check == GL_INVALID_FRAMEBUFFER_OPERATION)
			core_message += optional_string + " (code: GL_INVALID_FRAMEBUFFER_OPERATION)";
		
		if(error_check == GL_OUT_OF_MEMORY)
			core_message += optional_string + " (code: GL_OUT_OF_MEMORY)";
		
		throw std::logic_error(core_message + " ****");
	}
}

#endif //ERROR_LOGGER_H
