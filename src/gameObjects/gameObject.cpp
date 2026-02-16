#include "gameObjects/gameObject.h"

GameObject::GameObject()
	: children(), parent(), factory(nullptr), imguiName(), isActive(true), isActiveOverride(false),
	  imguiIsActive(isActive) {}

const std::vector<std::weak_ptr<GameObject>>& GameObject::GetChildren() const { return this->children; }

const int GameObject::GetChildCount() const { return this->children.size(); }

const std::weak_ptr<GameObject> GameObject::GetParent() const { return this->parent; }

void GameObject::SetParent(std::weak_ptr<GameObject> newParent) {
	if (!this->parent.expired()) {
		// Remove from the parent's children
		this->parent.lock()->DeleteChild(this->GetPtr());
	}

	this->parent = newParent;

	// If setting to root the parent is expired
	if (this->parent.expired()) {
		//Logger::Error("Tried to set expired parent.");
	} else {
		this->parent.lock()->AddChild(this->GetPtr());
	}


	Logger::Log("Set Parent.");
}

void GameObject::AddChild(std::weak_ptr<GameObject> newChild) {
	if (newChild.expired()) return;

	this->children.push_back(newChild);
}

void GameObject::DeleteChild(std::weak_ptr<GameObject> oldChild) {
	GameObject* oldChildPtr = oldChild.lock().get();
	for (size_t i = 0; i < this->children.size(); i++) {
		if (this->children[i].lock().get() == oldChildPtr) {
			this->children.erase(this->children.begin() + i);
		} else {
			Logger::Warn("Something went wrong with the delete function.");
		}
	}
}

void GameObject::Start() {}

void GameObject::Tick() {}

void GameObject::LateTick() {}

void GameObject::PhysicsTick() {
	// TO DO
}

void GameObject::LatePhysicsTick() {
	// TO DO
}

void GameObject::OnDestroy() {}

DirectX::XMMATRIX GameObject::GetGlobalWorldMatrix(bool inverseTranspose) const {
	if (this->parent.expired()) {
		return DirectX::XMMatrixIdentity();
	} else {
		return this->parent.lock()->GetGlobalWorldMatrix(inverseTranspose);
	}
}

DirectX::XMMATRIX GameObject::GetGlobalViewMatrix() const {
	if (this->parent.expired()) {
		return DirectX::XMMatrixIdentity();
	} else {
		return this->parent.lock()->GetGlobalViewMatrix();
	}
}

std::shared_ptr<GameObject> GameObject::GetPtr() {
	if (this->myPtr.expired()) {
		Logger::Error("How did this happen?");
	}

	return this->myPtr.lock();
	// return shared_from_this();
}

void GameObject::LoadFromJson(const nlohmann::json& data) {
	// Children are loaded by the sceneManager

	if (data.contains("name")) {
		this->SetName(data.at("name").get<std::string>());
	}
}

void GameObject::SaveToJson(nlohmann::json& data) {
	data["type"] = "GameObject";
	data["name"] = this->name;

	for (size_t i = 0; i < this->children.size(); i++) {
		this->children[i].lock()->SaveToJson(data["children"][i]);
	}
}

void GameObject::ShowInHierarchy() { 
	ImGui::InputText("Name", this->imguiName, sizeof(this->imguiName));

	ImGui::SameLine();

	if (ImGui::Button("Apply")) {
		this->name = this->imguiName; 
	}

	ImGui::Separator();
	ImGui::Text("Object details.");

	if (ImGui::Checkbox("Is Active", &this->imguiIsActive)) {
		SetActive(this->imguiIsActive);
	}

	if (ImGui::Button("Delete")) {
		this->factory->QueueDeleteGameObject(this->GetPtr());
	}

	ImGui::SameLine();
	if (this->factory->GetSelected().expired()) {
		if (ImGui::Button("Select")) {
			std::weak_ptr<GameObject> me = this->GetPtr();
			this->factory->SetSelected(me);
		}
	} else if (this->factory->GetSelected().lock().get() != this->GetPtr().get()) {
		if (ImGui::Button("Reparent selected")) {
			this->factory->GetSelected().lock()->SetParent(this->GetPtr());
			std::weak_ptr<GameObject> emptyObj = std::make_shared<GameObject>();
			this->factory->SetSelected(emptyObj);
		}
	} else {
		ImGui::Text("Selected");
	}
}

void GameObject::SetName(std::string newName) 
{ 
	this->name = newName;
	std::strncpy(this->imguiName, this->name.c_str(), sizeof(this->imguiName));
}

const std::string& GameObject::GetName() { return this->name; }

bool GameObject::IsActive() { return this->isActive && !this->isActiveOverride; }

void GameObject::SetActive(bool isActive) { 
	if (this->isActiveOverride) {
		Logger::Warn("Tried to set active when not possible");
		return;
	}

	this->isActive = isActive;
	SetActiveOverride(!this->isActive);
	this->isActiveOverride = false;
}

void GameObject::SetActiveOverride(bool isActive) { 
	this->isActiveOverride = isActive;
	for (auto& child : this->children) {
		child.lock()->SetActiveOverride(isActive);
	}
}
