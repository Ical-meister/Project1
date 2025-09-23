#pragma once
#include <string>
#include <vector>
#include "Mesh.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

class Model {
public:
    // Store all meshes
    std::vector<Mesh> meshes;

    // Constructor
    Model(const std::string &path);

    // Draw all meshes
    void Draw(unsigned int shaderID);

private:
    // Directory for locating textures
    std::string directory;

    // Loads a model with Assimp and stores the resulting meshes
    void loadModel(const std::string &path);

    // Process Assimp nodes and meshes
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    // Load textures from material
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};
