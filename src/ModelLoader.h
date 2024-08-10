
#include <vector>
#include <tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    bool LoadModel(const std::string& path);
private:
};