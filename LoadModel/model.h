#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "Shader.h"
#include "HalfEdge.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
//Mesh loadObj(string const& path,int n);




class Model
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    TriMesh* TMeshOri = new TriMesh();
    TriMesh* subDivisionMeshX = new TriMesh();
    TriMesh* subDivisionMeshY = new TriMesh();
    string directory;
    bool gammaCorrection;
    int                  LoopTimes = 0;
    int                  LastLoopTimes;



    // constructor, expects a filepath to a 3D model.
    Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {

        if (path.substr(path.find_last_of('.'), path.size() - 1) == ".obj") {
            loadObj(path);
            directory = path.substr(0, path.find_last_of('/'));

        }
        else if (path.substr(path.find_last_of('.'), path.size() - 1) == ".stl") {
            loadStl(path);
            directory = path.substr(0, path.find_last_of('/'));
        }
        else
        {
            loadModel(path);
        }


    }

    // draws the model, and thus all its meshes
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    void DrawLine(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DrawLine(shader);
    }

    void DrawPoint(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DrawPoint(shader);
    }

    void loopsubS(int times) {

        this->subDivisionMeshX = TriMesh::LoopSubdivison(this->TMeshOri);
        if (times > 1) {
            for (int i = 0; i < times - 1; i++) {
                this->subDivisionMeshY = TriMesh::LoopSubdivison(this->subDivisionMeshX);
                this->subDivisionMeshX = this->subDivisionMeshY;
            }
        }
        this->subDivisionMeshX->calculateNormal();
        meshes.clear();

        meshes.push_back(TriToMesh(this->subDivisionMeshX));

    }

    void loopsub() {

        this->subDivisionMeshX = TriMesh::LoopSubdivison(this->subDivisionMeshX);

        this->subDivisionMeshX->calculateNormal();
        meshes.clear();

        meshes.push_back(TriToMesh(this->subDivisionMeshX));

    }

    static Mesh TriToLine(TriMesh* T) {
        vector<Vertex> vertexs;
        vector<unsigned int> ind;

        auto& mv = T->Vertexs();

        for (int i = 0; i < mv.size(); i++) {
            Vertex newP;
            newP.Position = mv[i]->vcoord;
            newP.Normal = mv[i]->ncoord;
            vertexs.push_back(newP);
        }

        auto& mf = T->HalfEdges();

        for (int i = 0; i < mf.size(); i++) {
            HalfEdge* he = mf[i];
            ind.push_back(he->vert->id);
            ind.push_back(he->next->next->vert->id);
        }


        //auto& mv = T->Vertexs();
        //for (int i = 0; i < mf.size(); i++) {
        //    HalfEdge* f = mf[i];
        //    HalfEdge* he = f;
        //    Vertex newP;
        //        newP.Position = he->vert->vcoord;
        //        newP.Normal = he->vert->ncoord;
        //        ind.push_back(vi++);
        //        vertexs.push_back(newP);
        //        Vertex newP2;
        //        he = he->next->next;
        //        newP2.Position = he->vert->vcoord;
        //        newP2.Normal = he->vert->ncoord;
        //        ind.push_back(vi++);
        //        vertexs.push_back(newP2);


        //}
        vector<Texture>      tex;
        return Mesh(vertexs, ind, tex);
    
    }

    static Mesh TriToMesh(TriMesh* T) {
        vector<Vertex> vertexs;
        vector<unsigned int> ind;
        auto& mv = T->Vertexs();

        for (int i = 0; i < mv.size(); i++) {
            Vertex newP;
            newP.Position=mv[i]->vcoord;
            newP.Normal = mv[i]->ncoord;
            vertexs.push_back(newP);
        }

       // unsigned int vi = 0;
        auto& mf = T->Faces();
        for (int i = 0; i < mf.size(); i++) {
            Face* f = mf[i];
            HalfEdge* he = f->halfEdge;
            do {
                ind.push_back(he->vert->id);
                he = he->next;

            
            } while (he != f->halfEdge);

        }



        //auto& mv = T->Vertexs();
        //for (int i = 0; i < mf.size(); i++) {
        //    Face* f = mf[i];
        //    HalfEdge* he = f->halfEdge;

        //    do {
        //        Vertex newP;
        //        newP.Position = he->vert->vcoord;
        //        newP.Normal = he->vert->ncoord;
        //        ind.push_back(vi++);
        //        vertexs.push_back(newP);
        //        he = he->next;
        //    } while (he != f->halfEdge);
        //}
        vector<Texture>      tex;
        return Mesh(vertexs, ind, tex);

    }



private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

    void loadObj(string const& path)
    {
        vector<glm::vec3> verticesP;
        vector<glm::vec3> verticesN;
        vector<glm::uvec3> indicesP;
        vector<glm::uvec3> indicesN;
        int v_id = 0;
        vector<Texture> textures;
        fstream targetObj;
        string line;
        targetObj.open(path);
        if (!targetObj.is_open()) {
            cout << "Something wrong When opening ObjFile" << endl;
        }
        while (!targetObj.eof())
        {
            getline(targetObj, line);
            vector<string>parameters;
            
            string tailMark = " ";
            string ans = "";
            line = line.append(tailMark);
            for (int i = 0; i < line.length(); i++) {
                char ch = line[i];
                if (ch != ' ') {
                    ans += ch;
                }
                else {
                    parameters.push_back(ans);
                    ans = "";
                }
            }
            if (parameters[0] == "f")
            {
                unsigned int fpos[3], npos[3];
                for (int i = 1; i <= 3; i++) {

                    string temp = "";
                    temp = parameters[i].substr(0, parameters[i].find_first_of('/'));
                    //cout << parameters[i].substr(0, parameters[i].find_first_of('/')) << endl;
                    fpos[i - 1] = stoi(temp) - 1;
                    temp = parameters[i].substr(parameters[i].find_last_of('/') + 1, parameters[i].size());
                    npos[i - 1] = stoi(temp) - 1;
                }
                indicesP.push_back(glm::uvec3(fpos[0], fpos[1], fpos[2]));
                indicesN.push_back(glm::uvec3(npos[0], npos[1], npos[2]));

            }
            else if (parameters[0] == "v") {
                glm::vec3 position = { stof(parameters[1]), stof(parameters[2]), stof(parameters[3]) };

                verticesP.push_back(position);
                //为TriMesh类导入顶点
                TMeshOri->createVertex(position, v_id++);
            }
            else if (parameters[0] == "vn")
            {
                glm::vec3 normal = { stof(parameters[1]), stof(parameters[2]), stof(parameters[3]) };
                verticesN.push_back(normal);
            }

        }

        //if (LoopTimes != 0)
        //{
         //   loop(LoopTimes, verticesP, verticesN, indicesP, indicesN);
        //}
        targetObj.close();
        auto& mV = TMeshOri->Vertexs();
        for (int f = 0; f < indicesP.size(); f++) {
            Vert* v[3];
            v[0] = mV[indicesP[f][0]];
            v[1] = mV[indicesP[f][1]];
            v[2] = mV[indicesP[f][2]];
            TMeshOri->createFace(v);
        }
        TMeshOri->createBoundary();
        TMeshOri->calculateNormal();

        vector<Vertex> vertexs;
        vector<unsigned int> ind;
        unsigned int vi = 0;
        for (int i = 0; i < indicesP.size(); i++) {

            for (int j = 0; j < 3; j++) {
                Vertex newP;
                newP.Position = verticesP[indicesP[i][j]];
                newP.Normal = verticesN[indicesN[i][j]];
                ind.push_back(vi++);
                vertexs.push_back(newP);
            }



        }
        vector<Texture>      tex;
        this->meshes.push_back(Mesh(vertexs, ind, tex));

        // retrieve the directory path of the filepath
        //directory = path.substr(0, path.find_last_of('/'));

    }

    struct VKeyHashFuc
    {
        std::size_t operator()(const glm::vec3& key) const
        {
            return std::hash<float>()(key.x) + std::hash<float>()(key.y)+ std::hash<float>()(key.z);
        }
    };

    int loadStl(string const& path) {
        ifstream targetStl(path,ios::in|ios::binary);
        string line;
        char name[80];
        int v_id = 0;
        unsigned int triangles;
        //float normal;
        std::unordered_map<glm::vec3, int, VKeyHashFuc> hashmap_vid;
        if (!targetStl) {
            cout << "Open .stl went wrong" << endl;
            return 0;
        }
        targetStl.read(name, 80);
        targetStl.read((char*)&triangles, sizeof(unsigned int));
        if (triangles == 0)
            return -1;
        for (unsigned int i = 0; i < triangles; i++) {
            float XYZ[12];//4*3
            int face[3];
            targetStl.read((char*)XYZ, 12 * sizeof(float));
            for (int j = 1; j < 4; j++) {
                glm::vec3 v = { XYZ[j * 3],XYZ[j * 3 + 1],XYZ[j * 3 + 2] };
                if (hashmap_vid.find(v) != hashmap_vid.end()) {
                    //顶点已经存在
                    face[j - 1] = hashmap_vid[v];
                }
                else {
                    hashmap_vid[v] = v_id;
                    face[j - 1] = v_id;
                    TMeshOri->createVertex(v, v_id++); 
                    
                
                }
            }
            //生成面
            Vert* ve[3];
            auto& mV = TMeshOri->Vertexs();
            ve[0] = mV[face[0]];
            ve[1] = mV[face[1]];
            ve[2] = mV[face[2]];
            TMeshOri->createFace(ve);

            targetStl.read((char*)XYZ, 2);
        }
        targetStl.close();

        TMeshOri->createBoundary();
        TMeshOri->calculateNormal();

        this->meshes.push_back(TriToMesh(TMeshOri));
        return 0;

        

    }


 
    


};


unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}




#endif
