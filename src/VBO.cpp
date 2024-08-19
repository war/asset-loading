#include "VBO.h"

VBO::VBO(const std::vector<GLfloat>& tri_verts){
  glGenBuffers(1, &vbo);//create VBO
  glBindBuffer(GL_ARRAY_BUFFER, vbo);//bind to current VBO
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*tri_verts.size(), tri_verts.data(), GL_STATIC_DRAW);//fill VBO with vert data
}

void VBO::free(){
  glDeleteBuffers(1, &vbo);
}

void VBO::bind(){
  glBindBuffer(GL_ARRAY_BUFFER, vbo);//bind to current VBO
}

void VBO::unbind(){
  glBindBuffer(GL_ARRAY_BUFFER, 0);//unbind VBO
}
