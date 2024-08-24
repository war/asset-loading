#version 330 core

layout(location = 0) out vec4 fragColor;

// in vec3 vertColor;
in vec2 uvCoord;
in vec3 vertCoord;
in vec3 vertNormal;

in vec3 perpNormalVec;
in vec3 tangentVec;
in vec3 bitangentVec;

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;
uniform sampler2D metal_tex;

uniform vec3 cameraPos;

vec3 normal_mapping(){
  vec3 normal_map = normalize( texture2D(normal_tex, uvCoord).rgb * 2.f - vec3(1.f) );

  mat3 tbn = mat3(tangentVec, bitangentVec, perpNormalVec);

  vec3 normal = normalize(tbn * normal_map);

  return normal;
}

void main(){

  /* NORMAL OUTPUT */
  fragColor = texture2D(diffuse_tex, uvCoord);

//	fragColor = vec4(1.f, 0.f, 0.f, 1.f);
}
