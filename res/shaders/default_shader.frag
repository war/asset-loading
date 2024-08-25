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
uniform vec3 light_dir;
uniform float light_strength;
uniform float light_specular;
uniform vec3 light_color;
#define SPECULAR_BEAD_SHARPNESS 32.0
#define AMBIENT_INTENSITY 0.9 //ambient light

vec3 normal_mapping(){
  vec3 normal_map = normalize( texture2D(normal_tex, uvCoord).rgb * 2.f - vec3(1.f) );

  mat3 tbn = mat3(tangentVec, bitangentVec, perpNormalVec);

  vec3 normal = normalize(tbn * normal_map);

  return normal;
}

vec4 directionalLightFunc(){
  vec4 diffuseColor = texture(diffuse_tex, uvCoord).rgba;
  vec4 specularColor = vec4((texture(metal_tex, uvCoord).r + texture(metal_tex, uvCoord).g + texture(metal_tex, uvCoord).b)/3.f);

//   vec3 normal = normalize(vertNormal);
  vec3 normal = normal_mapping();
  vec3 light_dir = -normalize(light_dir);
  vec3 view_dir = normalize(vertCoord - cameraPos);
  vec3 light_reflect = reflect(light_dir, normal);

  float diffuse_part = clamp(dot(light_dir, normal), 0.f, 1.f) * light_strength + AMBIENT_INTENSITY ;

  /* specular calc */
  float spec_part = pow(clamp(dot(view_dir, light_reflect), 0.f, 1.f), SPECULAR_BEAD_SHARPNESS) * light_strength * light_specular;

  /* phong shading */
  vec4 final_color = (diffuseColor*diffuse_part) + (specularColor*spec_part) ;

//  return vec4(tangentVec, 1.f);
	return final_color * vec4(light_color, 1.f);
}


void main(){

  /* NORMAL OUTPUT */
//  fragColor = texture2D(diffuse_tex, uvCoord);
  fragColor = directionalLightFunc();

//	fragColor = vec4(1.f, 0.f, 0.f, 1.f);
}
