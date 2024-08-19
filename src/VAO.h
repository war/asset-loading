#ifndef VAO_HPP
#define VAO_HPP

#include <glad/glad.h>

/*
Vertex array buffer object class
*/

class VAO{
public:
  GLuint vao {}; //vertex array buffer

  VAO();
  void free();
  void bind();
  void unbind();
};

#endif // VAO_HPP
