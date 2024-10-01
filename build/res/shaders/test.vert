#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vColor;

out vec3 vertColor;

void main(){
  gl_Position = vec4(vertPos.x, vertPos.y, vertPos.z, 1.f);
  vertColor = vColor;
}
