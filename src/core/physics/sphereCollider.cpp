#include "core/physics/sphereCollider.h"
#include "core/physics/boxCollider.h"

SphereCollider::SphereCollider()
{
	this->SetType(ColliderType::SPHERE);
}

SphereCollider::~SphereCollider()
{
}

void SphereCollider::Tick()
{
	this->Collider::Tick();

	//force scale to the x-factor to ensure sphere shape
	// DirectX::XMVECTOR scale = this->transform.GetScale();
	// float diameter = scale.m128_f32[0];
	// scale.m128_f32[1] = diameter;
	// scale.m128_f32[2] = diameter;
	// //scale.m128_f32[3] = diameter;
	// this->transform.SetScale(scale);

	this->SetExtraCullingDistance((this->GetDiameter() * 0.5f) * (this->GetDiameter() * 0.5f));
}

void SphereCollider::Start()
{ 
	this->Collider::Start();

	this->SetExtraCullingDistance((this->GetDiameter() * 0.5f) * (this->GetDiameter() * 0.5f));
}

void SphereCollider::LoadFromJson(const nlohmann::json& data) 
{
	this->GameObject3D::LoadFromJson(data);

	if(data.contains("tag"))
	{
	this->SetTag(static_cast<Tag>(data.at("tag").get<int>())); //write enum as integer in json
	Logger::Log("tag was found in json: " + std::to_string(this->GetTag()));
	}
	else
	{
	Logger::Log("didn't find tag!!!");
	}

	if(data.contains("ignoreTag"))
	{
	this->SetIgnoreTag((Tag)data.at("ignoreTag").get<int>()); //write enum as integer in json
	Logger::Log("ignoreTag was found in json: " + std::to_string(this->GetIgnoreTag()));
	}

	if(data.contains("solid"))
	{
	this->SetSolid(data.at("solid").get<bool>()); //write enum as integer in json
	Logger::Log("solid was found in json: " + std::to_string(this->GetSolid()));
	}

	if(data.contains("dynamic"))
	{
	this->SetDynamic(data.at("dynamic").get<bool>()); //write enum as integer in json
	Logger::Log("dynamic was found in json: " + std::to_string(this->GetDynamic()));
	}
}

void SphereCollider::SaveToJson(nlohmann::json& data) 
{
	this->GameObject3D::SaveToJson(data);

	data["tag"] = this->GetTag();
	data["ignoreTag"] = this->GetIgnoreTag();
	data["solid"] = this->GetSolid();
	data["dynamic"] = this->GetDynamic();
}

bool SphereCollider::DoubleDispatchCollision(Collider* otherCollider, DirectX::XMFLOAT3& mtvAxis, float& mtvDistance) {
	return otherCollider->CollidesWithSphere(this, mtvAxis, mtvDistance);
}

void SphereCollider::SetPosition(DirectX::XMFLOAT3 newCenter)
{
	this->transform.SetPosition(DirectX::XMLoadFloat3(&newCenter));
}

void SphereCollider::SetDiameter(float diameter)
{
	DirectX::XMVECTOR sizeVector;
	sizeVector.m128_f32[0] = diameter;
	sizeVector.m128_f32[1] = diameter;
	sizeVector.m128_f32[2] = diameter;

	this->transform.SetScale(sizeVector);
}

float SphereCollider::GetDiameter()
{
	return this->transform.GetGlobalScale().m128_f32[0];
}

bool SphereCollider::CollidesWithBox(BoxCollider* box, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance)
{
	return this->BoxSphereCollision(box, this, resolveAxis, resolveDistance);
}

bool SphereCollider::CollidesWithSphere(SphereCollider* sphere, DirectX::XMFLOAT3& resolveAxis, float& resolveDistance)
{
	DirectX::XMVECTOR positionA = this->transform.GetGlobalPosition();
	DirectX::XMVECTOR positionB = sphere->transform.GetGlobalPosition();

	DirectX::XMVECTOR distanceVector = DirectX::XMVectorSubtract(positionB, positionA);
	float squaredDistance = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(distanceVector));

	float radiusSum = (this->GetDiameter() + sphere->GetDiameter()) * 0.5f;
	float squaredRadiusSum = radiusSum * radiusSum;

	if (squaredDistance > squaredRadiusSum)
	{
		resolveAxis = DirectX::XMFLOAT3(0, 1, 0);
		resolveDistance = 0;

		return false;
	}

	float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(distanceVector));
	DirectX::XMVECTOR axis;

	if (distance > 0.0001f)
	{
		axis = DirectX::XMVectorScale(distanceVector, 1.0f / distance); // normalized direction
	}
	else
	{
		axis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); //perfect overlap gets garbage axis
	}

	XMStoreFloat3(&resolveAxis, axis);
	resolveDistance = radiusSum - distance; //assuming both radiuses are positive

	return true;
}

//needs improved variable names
bool SphereCollider::IntersectWithRay(const Ray& ray, float& distance, float maxDistance) {
	

	Vector3D originToCenter = Vector3D(this->transform.GetGlobalPosition()) - ray.origin;
	
	float radius = this->GetDiameter() / 2.0f;
	if (originToCenter.Length() - radius > maxDistance) {
		return false;
	}
	float projectionDist = originToCenter * ray.direction;

	float originToCenterSquared = originToCenter * originToCenter;

	float radiusSquared = pow(radius, 2);

	if (projectionDist < 0 && originToCenterSquared > radiusSquared) return false;

	float Msquared = originToCenterSquared - pow(projectionDist, 2);
	
	float Qsquared = radiusSquared;
	if (Msquared > Qsquared) return false;

	float halfChordDist = sqrt(Qsquared - Msquared);

	if (originToCenterSquared > Qsquared)
		distance = projectionDist - halfChordDist;
	else
		distance = projectionDist + halfChordDist;

	return true;


}
