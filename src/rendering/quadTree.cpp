#include "rendering/quadTree.h"

QuadTree::QuadTree(DirectX::XMVECTOR volume, size_t maxDepth, size_t maxElements, DirectX::XMVECTOR center)
	: maxDepth(maxDepth), root(std::make_unique<QuadTree::Node>()) {

	// Define the center and extents of the bounding box
	DirectX::XMVECTOR extents = DirectX::XMVectorScale(volume, 0.5f);

	// Calculate min and max points
	DirectX::XMVECTOR minPoint = DirectX::XMVectorSubtract(center, extents);
	DirectX::XMVECTOR maxPoint = DirectX::XMVectorAdd(center, extents);

	// Create the bounding box
	DirectX::BoundingBox::CreateFromPoints(this->root->boundingBox, minPoint, maxPoint);
}

QuadTree::QuadTree(std::array<float, 3> pos1, std::array<float, 3> pos2, size_t maxDepth, size_t maxElements)
	: maxDepth(maxDepth), root(std::make_unique<QuadTree::Node>()) {
	DirectX::XMVECTOR minPoint = DirectX::XMVectorSet(pos1[0], pos1[1], pos1[2], 0.0f);
	DirectX::XMVECTOR maxPoint = DirectX::XMVectorSet(pos2[0], pos2[1], pos2[2], 0.0f);

	DirectX::BoundingBox::CreateFromPoints(this->root->boundingBox, minPoint, maxPoint);
}

void QuadTree::AddElement(std::weak_ptr<MeshObject> object) {
	if (object.expired()) {
		Logger::Warn("Tried to add expired object");
		return;
	}

	this->AddToNode(object.lock(), this->root, 0);
}

std::vector<std::weak_ptr<MeshObject>> QuadTree::GetVisibleElements(CameraObject& camera) {
	std::vector<std::weak_ptr<MeshObject>> out;
	std::unordered_set<MeshObject*> found;

	DirectX::BoundingFrustum viewFrustrum;
	DirectX::XMMATRIX proj = camera.GetProjectionMatrix();

	DirectX::BoundingFrustum::CreateFromMatrix(viewFrustrum, proj);
	DirectX::XMMATRIX view = camera.GetViewMatrix();
	DirectX::XMMATRIX world = DirectX::XMMatrixInverse(nullptr, view);
	viewFrustrum.Transform(viewFrustrum, world);

	this->collisionChecks = 0;
	this->CheckNode(viewFrustrum, this->root, out, found);

	ImGui::Begin("QuadTree Debug");
	ImGui::Text(std::format("intersections tests {}", this->collisionChecks).c_str());
	ImGui::End();

	return out;
}

void QuadTree::AddToNode(std::shared_ptr<MeshObject> element, std::unique_ptr<QuadTree::Node>& node,
						 size_t currentDepth) {

	if (!element->GetBoundingBox().Intersects(node->boundingBox)) {
		return;
	}

	if (currentDepth >= this->maxDepth) {
		node->elements.emplace_back(element);
		return;
	}

	bool isLeaf = (node->children[0] == nullptr);
	if (isLeaf) {
		if (node->elements.size() < this->maxElements) {
			node->elements.emplace_back(element);
			return;
		} else {
			this->SubdivideNode(node);

			// Redistribute the current elements to the appropriate child node
			for (int i = 0; i < 4; ++i) {
				for (auto& nodeElement : node->elements) {
					if (nodeElement.expired()) continue;

					this->AddToNode(nodeElement.lock(), node->children[i], currentDepth + 1);
				}
			}

			// Clear the current node's elements
			node->elements.clear();
		}
	}

	for (auto& child : node->children) {
		this->AddToNode(element, child, currentDepth + 1);
	}
}

void QuadTree::SubdivideNode(std::unique_ptr<Node>& node) {
	DirectX::XMVECTOR center = DirectX::XMLoadFloat3(&node->boundingBox.Center);
	DirectX::XMVECTOR extents = DirectX::XMLoadFloat3(&node->boundingBox.Extents);
	DirectX::XMVECTOR scale = DirectX::XMVectorSet(0.5f, 1.0f, 0.5f, 0.0f);
	DirectX::XMVECTOR halfExtents = DirectX::XMVectorMultiply(extents, scale);


	// Define offsets for the 4 child nodes
	const DirectX::XMVECTOR offsets[4] = {
		// -x -z quadrant
		DirectX::XMVectorSet(-DirectX::XMVectorGetX(halfExtents), 0.0f, -DirectX::XMVectorGetZ(halfExtents), 0.0f),
		// Bottom-right
		DirectX::XMVectorSet(DirectX::XMVectorGetX(halfExtents), 0.0f, -DirectX::XMVectorGetZ(halfExtents), 0.0f),
		// Top-left
		DirectX::XMVectorSet(-DirectX::XMVectorGetX(halfExtents), 0.0f, DirectX::XMVectorGetZ(halfExtents), 0.0f),
		// Top-right
		DirectX::XMVectorSet(DirectX::XMVectorGetX(halfExtents), 0.0f, DirectX::XMVectorGetZ(halfExtents), 0.0f)};

	for (int i = 0; i < 4; ++i) {
		node->children[i] = std::make_unique<Node>();

		// Calculate the center of the child node
		DirectX::XMVECTOR childCenter = DirectX::XMVectorAdd(center, offsets[i]);

		// Create the bounding box for the child node
		DirectX::BoundingBox::CreateFromPoints(node->children[i]->boundingBox,
											   DirectX::XMVectorSubtract(childCenter, halfExtents),
											   DirectX::XMVectorAdd(childCenter, halfExtents));
	}
}

void QuadTree::CheckNode(DirectX::BoundingFrustum& frustum, std::unique_ptr<Node>& node,
						 std::vector<std::weak_ptr<MeshObject>>& out, std::unordered_set<MeshObject*>& found) {
	
	bool isLeaf = (node->children[0] == nullptr);
	if (isLeaf && node->elements.size() <= 0) {
		return;
	}

	bool collision = frustum.Intersects(node->boundingBox);
	this->collisionChecks++;
	if (!collision) {
		return;
	}

	if (isLeaf) {
		for (auto& elementWeak : node->elements) {
			if (!elementWeak.expired()) {
				std::shared_ptr<MeshObject> element = elementWeak.lock();
				if (found.find(element.get()) == found.end()) {
					out.emplace_back(element);
					found.insert(element.get());
					/*bool inView = frustum.Intersects(element->GetBoundingBox());
					if (inView) {
						out.emplace_back(element);
						found.insert(element.get());
					}*/
				}
			}
		}
	} else {
		for (auto& child : node->children) {
			this->CheckNode(frustum, child, out, found);
		}
	}
}
