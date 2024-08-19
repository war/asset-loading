#include "VAO.h"

VAO::VAO(){
  glGenVertexArrays(1, &vao);//create VAO
}

void VAO::free(){
  glDeleteVertexArrays(1, &vao);
}

void VAO::bind(){
  glBindVertexArray(vao);//bind to current VAO
}

void VAO::unbind(){
  glBindVertexArray(0); //unbind VAO
}
