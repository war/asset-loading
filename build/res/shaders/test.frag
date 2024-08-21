#version 330 core

uniform vec3 inColor;
out vec4 fragColor;
in vec3 vertColor;

void main(){
  fragColor = vec4(vertColor.x, vertColor.y, vertColor.z, 1.f);
}
