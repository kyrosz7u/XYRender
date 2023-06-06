//
// Created by kyros on 5/27/23.
//

#ifndef XEXAMPLE_MODEL_H
#define XEXAMPLE_MODEL_H

#include "render/resource/render_mesh.h"
#include "transform.h"
#include <vector>
#include <string>
#include <memory>

using namespace RenderSystem;

namespace Scene
{
    class Model
    {
    public:
        RenderMeshPtr loaded_mesh;
        Transform     transform;

    public:
        Model() {}
        ~Model() {}
        void Tick();
        bool LoadModelFile(const std::string &mesh_path);
    private:
        void processModelNode(aiNode *node, const aiScene *scene);
        void processMesh(aiMesh *mesh, const aiScene *scene);

        uint32_t m_index_count{0};
        std::vector<RenderSystem::RenderSubmesh> m_submeshes;
    };
}
#endif //XEXAMPLE_MODEL_H
