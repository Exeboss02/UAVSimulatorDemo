#include "gameObjects/testEnemy.h"
#include <DirectXMath.h>

void TestEnemy::Start() {
	


}

void TestEnemy::Tick() { 
	if (!this->path.empty()) {

		if (this->currentPathIndex >= this->path.size()) {
			return;
		}

		if (DirectX::XMVector4NearEqual(this->GetGlobalPosition(), this->path[this->currentPathIndex]->GetGlobalPosition(), DirectX::XMVectorSet(1.f, 1.f, 1.f, 1.f))) {
			this->currentPathIndex++;
		}

		DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(this->path[this->currentPathIndex]->GetGlobalPosition(), this->GetGlobalPosition());
		direction = DirectX::XMVector3Normalize(direction);
		this->transform.Move(direction, this->movementSpeed * Time::GetInstance().GetDeltaTime());
	}
}
