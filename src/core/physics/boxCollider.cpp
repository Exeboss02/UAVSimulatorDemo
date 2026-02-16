#include "core/physics/boxCollider.h"
#include "core/physics/sphereCollider.h"
#include <limits>
#include <cmath>


BoxCollider::BoxCollider()
{
	this->type = ColliderType::BOX;
	this->satData.positionData = new DirectX::XMFLOAT3[8];

	this->satData.center = DirectX::XMFLOAT3(0, 0, 0);
	this->satData.nrOfPositions = 8;

	DirectX::XMStoreFloat3(&this->axis[0], this->GetGlobalRight());
	DirectX::XMStoreFloat3(&this->axis[1], this->GetGlobalUp());
	DirectX::XMStoreFloat3(&this->axis[2], this->GetGlobalForward());
	this->satData.nrOfNormals = 3;
	this->satData.normalData = this->axis;

	this->BuildCornersArray(this->satData.positionData);
}

BoxCollider::~BoxCollider()
{
	delete[] this->satData.positionData;
}

void BoxCollider::Tick()
{
	this->Collider::Tick();

	//update normals
	DirectX::XMStoreFloat3(&this->axis[0], this->GetGlobalRight());
	DirectX::XMStoreFloat3(&this->axis[1], this->GetGlobalUp());
	DirectX::XMStoreFloat3(&this->axis[2], this->GetGlobalForward());
	this->satData.normalData = this->axis;

	this->BuildCornersArray(this->satData.positionData);
	DirectX::XMStoreFloat3(&this->satData.center, this->GetGlobalPosition());
}

void BoxCollider::LoadFromJson(const nlohmann::json& data)
{
	this->GameObject3D::LoadFromJson(data);

	DirectX::XMStoreFloat3(&this->satData.center, this->GetGlobalPosition()); //check positions later
	this->BuildCornersArray(this->satData.positionData);

	Logger::Log("------------------BEFORE TAGS-----------------");

	 if(data.contains("tag"))
	 {
	 	this->tag = static_cast<Tag>(data.at("tag").get<int>()); //write enum as integer in json
	 	Logger::Log("tag was found in json: " + std::to_string(this->tag));
	 }
	 else
	 {
	 	Logger::Log("didn't find tag!!!");
	 }

	 if(data.contains("targetTag"))
	 {
	 	this->targetTag = (Tag)data.at("targetTag").get<int>(); //write enum as integer in json
	 	Logger::Log("targetTag was found in json: " + std::to_string(this->targetTag));
	 }

	 if(data.contains("solid"))
	 {
	 	this->solid = data.at("solid").get<bool>(); //write enum as integer in json
	 	Logger::Log("solid was found in json: " + std::to_string(this->solid));
	 }

	 if(data.contains("dynamic"))
	 {
	 	this->dynamic = data.at("dynamic").get<bool>(); //write enum as integer in json
	 	Logger::Log("dynamic was found in json: " + std::to_string(this->dynamic));
	 }
}

void BoxCollider::SaveToJson(nlohmann::json& data)
{
	this->GameObject3D::SaveToJson(data);

	data["tag"] = this->tag;
	data["targetTag"] = this->targetTag;
	data["solid"] = this->solid;
	data["dynamic"] = this->dynamic;
}

bool BoxCollider::CollidesWithBox(class BoxCollider* box, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance)
{
	bool collision = SAT(this->satData, box->satData, resolveAxis, resolveDistance);

	return collision;
}

bool BoxCollider::CollidesWithSphere(SphereCollider* sphere, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance)
{
	return this->BoxSphereCollision(this, sphere, resolveAxis, resolveDistance);
}

void BoxCollider::BuildCornersArray(DirectX::XMFLOAT3*& positionArray)
{
	using namespace DirectX;

	XMMATRIX worldMatrix = this->GetGlobalWorldMatrix(false); //iverse transpose?
	//worldMatrix = XMMatrixTranspose(worldMatrix);

	for (int i = 0; i < 8; i++)
	{
		XMVECTOR localPosition = XMLoadFloat3(&localBoxCorners[i]);
		XMVECTOR worldPosition = XMVector3TransformCoord(localPosition, worldMatrix);

		XMStoreFloat3(&positionArray[i], worldPosition);
	}
}

bool BoxCollider::DoubleDispatchCollision(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance)
{
	return otherCollider->CollidesWithBox(this, mtvAxis, mtvDistance);
}

void BoxCollider::SetExtents(DirectX::XMFLOAT3 extents)
{
	this->transform.SetScale(DirectX::XMLoadFloat3(&extents));
	this->BuildCornersArray(this->satData.positionData);

	if (extents.x < this->shortestExtent) this->shortestExtent = extents.x;
	if (extents.y < this->shortestExtent) this->shortestExtent = extents.y;
	if (extents.z < this->shortestExtent) this->shortestExtent = extents.z;

	// Logger::Log("----------SAT-DATA---------");

	// for (int i = 0; i < 8; i++)
	// {
	// 	Logger::Log("positions: " + std::to_string(this->satData.positionData[i].x), +", " + std::to_string(this->satData.positionData[i].y) + std::to_string(this->satData.positionData[i].z));
	// }

	// Logger::Log("----------||---------");
}

DirectX::XMFLOAT3 BoxCollider::GetExtents()
{
	DirectX::XMFLOAT3 extents = {};
	DirectX::XMStoreFloat3(&extents, this->GetGlobalScale());
	
	return extents;
}

void BoxCollider::SetPosition(DirectX::XMFLOAT3 newCenter)
{
	this->transform.SetPosition(DirectX::XMLoadFloat3(&newCenter));
	this->satData.center = newCenter;
	this->BuildCornersArray(this->satData.positionData);
}

void BoxCollider::SetRotation(DirectX::XMFLOAT3 newRotation)
{
	this->transform.SetRotationQuaternion(DirectX::XMLoadFloat3(&newRotation)); //quaternion?

	DirectX::XMFLOAT3 rotation = newRotation;
	DirectX::XMVECTOR rotationVector = DirectX::XMLoadFloat3(&rotation);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYawFromVector(rotationVector);
	DirectX::XMStoreFloat3(&this->axis[0], rotationMatrix.r[0]);
	DirectX::XMStoreFloat3(&this->axis[1], rotationMatrix.r[1]);
	DirectX::XMStoreFloat3(&this->axis[2], rotationMatrix.r[2]);

	this->satData.normalData = this->axis;
	this->BuildCornersArray(this->satData.positionData);
}

bool BoxCollider::IntersectWithRay(const Ray& ray, float& distance, float maxDistance) { 
	
	float EPSILON = 0.000001;
	
	float tMAX = std::numeric_limits<float>::max();
	float tMIN = 0.0f;

	Vector3D originToCenter = Vector3D(this->GetGlobalPosition()) - ray.origin;

	float dimensions[3] = {
		this->GetExtents().x,
		this->GetExtents().y,
		this->GetExtents().z
	};
	
	for (int i = 0; i < 3; i++) {
		float axisOffset = Vector3D(this->axis[i]) * originToCenter;
		float rayProj = Vector3D(this->axis[i]) * ray.direction;
		
		if (std::abs(rayProj) > EPSILON) {
			float t1 = (axisOffset + dimensions[i]) / rayProj;
			float t2 = (axisOffset - dimensions[i]) / rayProj;

			if (t1 > t2) std::swap(t1, t2);
			if (t1 > tMIN) tMIN = t1;
			if (t2 < tMAX) tMAX = t2;
			if (tMIN > tMAX) return false;
			if (tMAX < 0) return false;

		} else if (-axisOffset - dimensions[i] > 0 || -axisOffset + dimensions[i] < 0)
			return false;
	}

	if (tMIN > 0) {

		if (maxDistance > tMIN) {
			distance = tMIN;
			return true;
		}
		distance = tMIN;
		return false;
	}
	if (maxDistance > tMAX) {
		distance = tMAX;
		return true;
	}
	distance = tMAX;
	return false;
}