#version 330 core

layout(location = 0) in vec3 vertPos; //vertex pos
layout(location = 1) in vec2 uvVertCoord; //vertex UV's
layout(location = 2) in vec3 vNormal; //vertex normals

out vec2 uvCoord;
out vec3 vertCoord;
out vec3 vertNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main(){
  /* transform verts from Local->World->View->Clip space */
  gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertPos.xyz, 1.f);
  // vertColor = vColor;
  uvCoord = uvVertCoord;
  vertCoord = vec3( (modelMatrix * vec4(vertPos.x, vertPos.y, vertPos.z, 1.f)).xyz );//transform local -> global coord
  vertNormal = vNormal;
}
