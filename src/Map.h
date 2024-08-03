#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

class Map {
public:
    Map(int size, float spacing);
    ~Map();

    void draw(Shader& shader);

private:
    void createGrid();

    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int size;
    float spacing;
};