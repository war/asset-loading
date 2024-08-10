#include "ModelLoader.h"

ModelLoader::ModelLoader() {}
ModelLoader::~ModelLoader() {}

bool ModelLoader::LoadModel(const std::string& path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!ret) {
        std::cout << "Failed to load glTF: " << path << std::endl;
        return false;
    }

    std::cout << "Model loaded successfully. Number of meshes: " << model.meshes.size() << std::endl;

    return true;
}