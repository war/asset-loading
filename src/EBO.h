#ifndef EBO_HPP
#define EBO_HPP


#include <vector>

#include <glad/glad.h>

/*
Index buffer class -- allows using the same set of indices to render polygons 
*/

class EBO{
public:
  GLuint ebo {}; //index buffer

	explicit EBO() = default;
  EBO(const std::vector<GLuint>& tri_indices);
  void free();
  void bind();
  void unbind();
};



#endif // EBO_HPP
