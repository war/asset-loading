#version 330 core

layout(location = 0) in vec3 vertPos; //vertex pos
layout(location = 1) in vec2 uvVertCoord; //vertex UV's
layout(location = 2) in vec3 vNormal; //vertex normals
layout(location = 3) in vec3 vTangent; //tangent
layout(location = 4) in vec3 vBiTangent; //bitangent
layout(location = 5) in vec4 joints_0; //JOINTS_0
layout(location = 6) in vec4 weights_0; //WEIGHTS_0

out vec2 uvCoord;
out vec3 vertCoord;
out vec3 vertNormal;

out vec3 tangentVec;
out vec3 bitangentVec;
out vec3 perpNormalVec;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

struct inverseBindMatrixStruct{
  mat4 matrix;
};

uniform inverseBindMatrixStruct inverseBindMatrixArray[64];

uniform int isSkinned;

void main(){

  if(isSkinned == 1){
      mat4 skinnedMatrix =
                        ( weights_0.x * inverseBindMatrixArray[int(joints_0.x)].matrix ) +
                        ( weights_0.y * inverseBindMatrixArray[int(joints_0.y)].matrix ) +
                        ( weights_0.z * inverseBindMatrixArray[int(joints_0.z)].matrix ) +
                        ( weights_0.w * inverseBindMatrixArray[int(joints_0.w)].matrix );
  
    
    gl_Position = projMatrix * viewMatrix * modelMatrix * skinnedMatrix * vec4(vertPos.xyz, 1.f);
  }
  else{
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertPos.xyz, 1.f);
  }

  
  
  uvCoord = uvVertCoord;
  vertCoord = vec3( (modelMatrix * vec4(vertPos.x, vertPos.y, vertPos.z, 1.f)).xyz );//transform local -> global coord
  vertNormal = vNormal;
  
  
  tangentVec = normalize(vec3( (modelMatrix * vec4(vTangent.xyz, 0.f)).xyz ));
  bitangentVec = normalize(vec3( (modelMatrix * vec4(vBiTangent.xyz, 0.f)).xyz ));
  perpNormalVec = normalize(cross(bitangentVec, tangentVec));
}
