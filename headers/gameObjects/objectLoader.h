#pragma once

#include "gameObjects/mesh.h"
#include "gameObjects/meshObjData.h"
#include "rendering/genericMaterial.h"
#include "rendering/indexBuffer.h"
#include "rendering/vertex.h"
#include "rendering/vertexBuffer.h"

#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
#include <filesystem>
#include <string>

struct MeshLoadData {
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GenericMaterial>> materials;
	std::vector<std::shared_ptr<Texture>> textures;
	std::vector<MeshObjData> meshData;
};

class ObjectLoader {
public:
	ObjectLoader(std::filesystem::path basePath = "../../../../assets");
	~ObjectLoader();

	/// <summary>
	/// Loads a gltf file from basepath / localpath
	/// </summary>
	/// <param name="localpath">path to file from basePath</param>
	/// <param name="meshLoadData"> OUT data parsed </param>
	/// <param name="device"> device to create textures and other gpu data </param>
	/// <returns></returns>
	bool LoadGltf(std::filesystem::path localpath, MeshLoadData& meshLoadData, ID3D11Device* device);

private:
	/// <summary>
	/// Loads Verticies and normals into dest starting from offset
	/// </summary>
	/// <param name="asset"></param>
	/// <param name="primitive"></param>
	/// <param name="dest"></param>
	/// <param name="offset"></param>
	/// <returns>Number of vertices added</returns>
	size_t LoadVerticiesAndNormals(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive,
								   std::vector<Vertex>& dest, uint32_t offset);

	/// <summary>
	/// Loads UV for vertices
	/// </summary>
	/// <param name="asset"></param>
	/// <param name="primitive"></param>
	/// <param name="dest"></param>
	/// <param name="offset"></param>
	/// <returns> If UV load was successful</returns>
	bool LoadUV(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive, std::vector<Vertex>& dest,
				size_t offset);

	/// <summary>
	/// Loads indices into dest vector, offset is incremented with number of indices loaded
	/// </summary>
	/// <param name="asset"></param>
	/// <param name="primitive"></param>
	/// <param name="dest"></param>
	/// <param name="offset"></param>
	/// <returns> If indices load was successful</returns>
	bool LoadIndices(fastgltf::Asset& asset, const fastgltf::Primitive& primitive, std::vector<uint32_t>& dest,
					 uint32_t& offset, size_t vertexOffset);

	/// <summary>
	/// Loads a texture
	/// </summary>
	/// <param name="asset"></param>
	/// <param name="primitive"></param>
	/// <param name="device"></param>
	/// <returns>texture pointer</returns>
	ID3D11ShaderResourceView* LoadTexture(fastgltf::Asset& asset, fastgltf::TextureInfo& textureInfo,
										  ID3D11Device* device);

	std::filesystem::path basePath;
};