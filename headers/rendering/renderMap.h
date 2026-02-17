#pragma once

#include <unordered_map>
#include "gameObjects/mesh.h"
#include "gameObjects/meshObjData.h"
#include <string>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

struct RenderMap {
public:
	struct WorldMatrixBufferContainer {
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT4X4 worldMatrixInversedTransposed;
	};

	struct RenderMapMaterial {
		// Each material has a list of objects
		std::vector<RenderMap::WorldMatrixBufferContainer> objects;

		std::shared_ptr<BaseMaterial> material;
	};


	struct RenderMapSubmesh {
		// Each submesh has a set of materials
		std::unordered_map<std::string, RenderMap::RenderMapMaterial> materials;
	};

	struct RenderMapMesh {
		// Each mesh has submeshes
		std::vector<RenderMapSubmesh> submeshes;

		std::shared_ptr<Mesh> mesh;
	};


	std::unordered_map<std::string, RenderMap::RenderMapMesh> meshes;
};
