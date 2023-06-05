//
// Created by kyros on 5/27/23.
//

#ifndef XEXAMPLE_MODEL_H
#define XEXAMPLE_MODEL_H

#include "render/resource/render_mesh.h"
#include <vector>
#include <string>
#include <memory>

using namespace RenderSystem;

namespace Scene
{
    class Model
    {
    public:
        RenderMeshPtr m_loaded_mesh;
    public:
        Model() {}
        ~Model() {}
        bool loadModelFile(const std::string &mesh_path);
        void processModelNode(aiNode *node, const aiScene *scene);
        void processMesh(aiMesh *mesh, const aiScene *scene);

    private:
        uint32_t m_index_count{0};
        std::vector<RenderSystem::RenderSubmesh> m_submeshes;
    };
}
#endif //XEXAMPLE_MODEL_H
