#pragma once

#include <DirectXCollision.h>
#include <DirectXMath.h>
#include "gameObjects/meshObject.h"
#include <memory>
#include "gameObjects/cameraObject.h"
#include <unordered_set>

class QuadTree {
public:
	QuadTree(DirectX::XMVECTOR volume, size_t maxDepth, size_t maxElements, DirectX::XMVECTOR center);
	QuadTree(std::array<float, 3> pos1, std::array<float, 3> pos2, size_t maxDepth, size_t maxElements);

	void AddElement(std::weak_ptr<MeshObject> object);


	std::vector<std::weak_ptr<MeshObject>> GetVisibleElements(CameraObject& camera, bool checkIndividualObjects = true, bool onlyShadowCasters = true);

	std::vector<std::weak_ptr<MeshObject>> GetAllElements();

private:
	struct Node {
		DirectX::BoundingBox boundingBox;
		std::vector<std::weak_ptr<MeshObject>> elements;
		std::array<std::unique_ptr<Node>, 4> children;
	};

	void AddToNode(std::shared_ptr<MeshObject> elementAddress, std::unique_ptr<Node>& node, size_t currentDepth);
	void SubdivideNode(std::unique_ptr<Node>& node);
	void CheckNode(DirectX::BoundingFrustum& frustum, std::unique_ptr<Node>& node,
				   std::vector<std::weak_ptr<MeshObject>>& out, std::unordered_set<MeshObject*>& found);

	std::vector<std::weak_ptr<MeshObject>> allElements;
	
	size_t maxDepth;
	size_t maxElements;
	std::unique_ptr<Node> root;

	size_t collisionChecks = 0;
	bool checkIndividual = true;
	bool onlyShadowCasters = true;
};