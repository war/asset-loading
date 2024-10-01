#version 330 core

layout(location = 0) out vec4 fragColor;

// in vec3 vertColor;
in vec2 uvCoord;
in vec3 vertCoord;
in vec3 vertNormal;

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;
uniform sampler2D metal_tex;

void main(){

  /* NORMAL OUTPUT */
  fragColor = texture2D(diffuse_tex, uvCoord);

//	fragColor = vec4(1.f, 0.f, 0.f, 1.f);
}
