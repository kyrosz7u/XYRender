//
// Created by kyros on 5/27/23.
//

#ifndef XEXAMPLE_MODEL_H
#define XEXAMPLE_MODEL_H

#include "render/resource/render_mesh.h"
#include <vector>
#include <string>

namespace Scene
{
    class Model
    {
    public:
        Model();
        ~Model();
        bool loadModelFile(const std::string &mesh_path);
        void processModelNode(aiNode *node, const aiScene *scene);
        void processMesh(aiMesh *mesh, const aiScene *scene);
    public:
        std::vector<RenderSystem::RenderMeshPtr> m_meshes;
    };
}
#endif //XEXAMPLE_MODEL_H
