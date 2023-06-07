//
// Created by kyros on 5/27/23.
//

#ifndef XEXAMPLE_MODEL_H
#define XEXAMPLE_MODEL_H

#include "render/resource/render_mesh.h"
#include "render/resource/render_texture.h"
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
        Transform                 transform;
        RenderMeshPtr             mesh_loaded;
        std::vector<Texture2DPtr> textures_loaded;
    public:
        Model()
        {}

        ~Model()
        {}

        void Tick();

        bool LoadModelFile(const std::string &mesh_path);

    private:
        void processModelNode(aiNode *node, const aiScene *scene);

        void processMesh(aiMesh *mesh, const aiScene *scene);

        uint32_t                                 m_index_count{0};
        std::vector<RenderSystem::RenderSubmesh> m_submeshes;

        void loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName);
    };
}
#endif //XEXAMPLE_MODEL_H
