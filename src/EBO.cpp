#include "EBO.h"

EBO::EBO(const std::vector<GLuint>& tri_indices){
  glGenBuffers(1, &ebo);//create EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);//bind EBO
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*tri_indices.size(), tri_indices.data(), GL_STATIC_DRAW);//fill EBO with index data
}

void EBO::free(){
  glDeleteBuffers(1, &ebo); //delete
}

void EBO::bind(){
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);//bind EBO
}

void EBO::unbind(){
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//unbind EBO
}
