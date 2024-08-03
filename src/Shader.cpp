#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() : id(0) {}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) : id(0) {
    loadFromFile(vertexPath, fragmentPath);
}

Shader::~Shader() {
    glDeleteProgram(id);
}

bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;

    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(shaderDir + vertexPath);
        fShaderFile.open(shaderDir + fragmentPath);

        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        return false;
    }

    GLuint vertex, fragment;

    if (!compileShader(vertexCode, GL_VERTEX_SHADER, vertex))
        return false;

    if (!compileShader(fragmentCode, GL_FRAGMENT_SHADER, fragment))
        return false;

    id = glCreateProgram();

    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return true;
}

void Shader::use() const {
    glUseProgram(id);
}

void Shader::setBool(const std::string& name, bool value) {
    glUniform1i(getUniformLocation(name), (int)value);
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

bool Shader::compileShader(const std::string& source, GLenum shaderType, GLuint& shaderID) {
    shaderID = glCreateShader(shaderType);
    const char* src = source.c_str();

    glShaderSource(shaderID, 1, &src, NULL);
    glCompileShader(shaderID);

    GLint success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    return true;
}

bool Shader::isValid() const {
    return id != 0;
}

GLint Shader::getUniformLocation(const std::string& name) {
    if (uniformLocationCache.find(name) != uniformLocationCache.end()) {
        return uniformLocationCache[name];
    }

    GLint location = glGetUniformLocation(id, name.c_str());

    uniformLocationCache[name] = location;

    return location;
}