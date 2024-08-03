#include "Map.h"
#include <glm/gtc/matrix_transform.hpp>

Map::Map(int size, float spacing) : size(size), spacing(spacing) {
    createGrid();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Map::~Map() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Map::createGrid() {
    for (int i = 0; i <= size; ++i) {
        for (int j = 0; j <= size; ++j) {
            float x = (i - size / 2) * spacing;
            float z = (j - size / 2) * spacing;

            vertices.push_back(x);
            vertices.push_back(0.0f);  // y is always 0
            vertices.push_back(z);
        }
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int topLeft = i * (size + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * (size + 1) + j;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

void Map::draw(Shader& shader) {
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}