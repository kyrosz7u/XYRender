//
// Created by kyros on 5/27/23.
//

#include "scene/model.h"

using namespace Scene;

bool Model::loadModelFile(const std::string &mesh_path)
{
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(mesh_path,
                                              aiProcess_Triangulate |
                                              aiProcess_CalcTangentSpace| // 计算uv镜像
                                              aiProcess_ConvertToLeftHanded);

    if (pScene == nullptr)
        return false;

    processModelNode(pScene->mRootNode, pScene);
    return true;
}

void Model::processModelNode(aiNode *node, const aiScene *scene)
{
    // 处理节点所有的网格（如果有的话）
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }
    // 接下来对它的子节点重复这一过程
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processModelNode(node->mChildren[i], scene);
    }
}

void Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
//    vector<Vertex> vertices;
//    vector<unsigned int> indices;
//    vector<Texture> textures;

    RenderSystem::RenderMesh render_mesh;

    render_mesh.m_name = mesh->mName.C_Str();
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        RenderSystem::VulkanMeshVertexPostition vertex_position;
        RenderSystem::VulkanMeshVertexNormal vertex_normal;
        RenderSystem::VulkanMeshVertexTexcoord vertex_texcoord;

        // 处理顶点位置、法线和纹理坐标
        vertex_position.position = Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex_normal.normal= Vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        Vector3 tangent;
        Vector3 bitangent;

        tangent = Vector3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        bitangent = Vector3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

        vertex_normal.tangent = Vector4(tangent.x, tangent.y, tangent.z, 1.0f);

        // 计算切线空间手性
        if(vertex_normal.normal.crossProduct(tangent).dotProduct(bitangent) < 0.0f)
        {
            vertex_normal.tangent.w = -1.0f;
        }
        else
        {
            vertex_normal.tangent.w = 1.0f;
        }


        if(mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            vertex_texcoord.texCoord.x = mesh->mTextureCoords[0][i].x;
            vertex_texcoord.texCoord.y = mesh->mTextureCoords[0][i].y;
        }
        else
            vertex_texcoord.texCoord = Vector2::ZERO;

        render_mesh.m_positions.push_back(vertex_position);
        render_mesh.m_normals.push_back(vertex_normal);
        render_mesh.m_texcoords.push_back(vertex_texcoord);
    }
    // 处理索引
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            render_mesh.m_indices.push_back(face.mIndices[j]);
    }
    // 处理材质
    if(mesh->mMaterialIndex >= 0)
    {

    }
    m_meshes.push_back(std::make_shared<RenderSystem::RenderMesh>(render_mesh));
}




