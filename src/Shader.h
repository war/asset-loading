#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <glm/ext/matrix_float4x4.hpp>

class Shader {
public:
    Shader();
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);

    bool isValid() const;

    // TODO: create a resource manager to deal with paths
    std::string const shaderDir = "res/shaders/";

    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    GLuint getShaderID() {
        return id;
    }

private:
    GLuint id;

    std::unordered_map<std::string, GLint> uniformLocationCache;

    bool compileShader(const std::string& source, GLenum shaderType, GLuint& shaderID);
    GLint getUniformLocation(const std::string& name);
};