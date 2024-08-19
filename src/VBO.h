#ifndef VBO_HPP
#define VBO_HPP

#include <vector>

#include <glad/glad.h>

/*
Vertex buffer object class
*/

class VBO{
public:
  GLuint vbo {}; //vertex buffer

	explicit VBO() = default;
  VBO(const std::vector<GLfloat>& tri_verts);
  void free();
  void bind();
  void unbind();
};


#endif // VBO_HPP
