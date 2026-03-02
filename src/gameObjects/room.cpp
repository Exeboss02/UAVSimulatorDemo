#include "gameObjects/room.h"
#include "UI/button.h"
#include "UI/canvas.h"
#include "UI/canvasObject.h"
#include "UI/interactionPrompt.h"
#include "core/assetManager.h"
#include "game/crosshair.h"
#include "game/events.h"
#include "game/gameManager.h"
#include "game/player.h"
#include "game/resourceGenerator.h"
#include "gameObjects/mine.h"
#include "gameObjects/spaceShipObj.h"
#include "gameObjects/spotlightObject.h"
#include "gameObjects/turret.h"
#include "rendering/renderQueue.h"
#include "utilities/time.h"
#include <numbers>
#include <format>

static const std::array<std::array<int, 2>, 4> wallpositions = {
	std::array<int, 2>({0, 1}),
	std::array<int, 2>({1, 0}),
	std::array<int, 2>({0, -1}),
	std::array<int, 2>({-1, 0}),
};

std::array<int, 2> Room::GetNeighborOffset(Room::WallIndex wallIndex) { return wallpositions[wallIndex]; }

void Room::CreateRoom(WallIndex wallIndex) {
	Logger::Log("Creating room");
	auto parentWeak = this->GetParent();
	if (parentWeak.expired()) {
		Logger::Error("Room parent is dead, how in the funk?");
		return;
	}

	auto parent = std::static_pointer_cast<SpaceShip>(parentWeak.lock());

	auto neighOffset = Room::GetNeighborOffset(wallIndex);
	Logger::Log("Position: ", this->pos[0], " ", this->pos[1]);
	parent->CreateRoom(this->pos[0] + neighOffset[0], this->pos[1] + neighOffset[1]);
}

void Room::SetPosition(size_t x, size_t y) { this->pos = {x, y}; }

void Room::Start() {
	Logger::Warn("room size ", this->size);

	DirectX::XMVECTOR position = this->transform.GetGlobalPosition();

	if(!(position.m128_f32[0] == 140 && position.m128_f32[2] == 0)) //for not playing build sound in the starting room
	{
		SoundClip* buildClip = AssetManager::GetInstance().GetSoundClip("Build1.wav");
		this->speaker = this->factory->CreateStaticGameObject<SoundSourceObject>();
		this->speaker.lock()->SetParent(this->GetPtr());
		this->speaker.lock()->SetRandomPitch(0.7f, 1.0f);
		this->speaker.lock()->SetGain(0.8f);
		this->speaker.lock()->Play(buildClip);
	}

	this->SetName("ROOM " + std::to_string(this->factory->GetNextID()));

	{
		auto meshobj = this->factory->CreateStaticGameObject<MeshObject>();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room2.glb:Mesh_0");

		meshobj->SetMesh(meshdata);

		this->roof = meshobj;
	}
	{
		auto meshobj = this->factory->CreateStaticGameObject<MeshObject>();

		meshobj->SetParent(this->GetPtr());

		MeshObjData meshdata = AssetManager::GetInstance().GetMeshObjData("SpaceShip/room2.glb:Mesh_4");
		meshobj->SetMesh(meshdata);

		this->floor = meshobj;
	}
	Logger::Warn("i am a room at x:", this->pos[0], " y: ", this->pos[1]);
	auto parentWeak = this->GetParent();
	if (parentWeak.expired()) {
		Logger::Error("Room parent is dead, how in the funk?");
		return;
	}

	auto parent = std::static_pointer_cast<SpaceShip>(parentWeak.lock());

	size_t maxX = parent->SHIP_MAX_SIZE_X;
	size_t maxY = parent->SHIP_MAX_SIZE_Y;
	for (size_t i = 0; i < 4; i++) {

		auto meshobj = this->factory->CreateStaticGameObject<Wall>();

		meshobj->SetParent(this->GetPtr());

		meshobj->transform.SetRotationRPY(0, 0, i * std::numbers::pi / 2);

		bool edgeWall = false;
		if (this->pos[1] == 0 && i == 2 || this->pos[1] == maxY && i == 0) {
			edgeWall = true;
		}

		if (this->pos[0] == maxX && i == 1 || this->pos[0] == 0 && i == 3) {
			edgeWall = true;
		}
		meshobj->SetWallState(Room::WallState::window, edgeWall);

		meshobj->SetWAllIndex(i);

		this->walls[i] = meshobj;
	}
	auto buildCollider = this->factory->CreateStaticGameObject<BoxCollider>();
	//fixing scale issue
	DirectX::XMFLOAT3 shipScale;
		DirectX::XMStoreFloat3(&shipScale, this->transform.GetScale());
	buildCollider->transform.SetScale(DirectX::XMVECTOR({(1.0f / shipScale.x), (0.5f / shipScale.y), (1.0f / shipScale.z)}));
	buildCollider->transform.SetPosition({0, 1, 0});
	buildCollider->SetParent(this->GetPtr());
	buildCollider->SetSolid(false);
	buildCollider->SetName("BuildCollider" + std::to_string(this->factory->GetNextID()));
	buildCollider->SetTag(Tag::INTERACTABLE);
	buildCollider->SetIgnoreTag(Tag::PLAYER);
	// Maybe tweak positionW
	this->buildSlot = buildCollider;
	buildCollider->SetOnInteract([&](std::shared_ptr<Player> p) {
		if (this->builtObject.expired() && !GameManager::GetInstance()->GetInCombat()) {
			this->ShowBuildMenu(p);
		}
	});
	buildCollider->SetOnHover([&] { this->Hover(); });

	auto spotLight = this->factory->CreateGameObjectOfType<SpotlightObject>().lock();
	spotLight->SetParent(this->GetPtr());
	spotLight->transform.SetPosition({0, 4.5, 0});
	spotLight->transform.SetRotationRPY(0, std::numbers::pi / 2, 0);
	spotLight->SetAngle(120.);
	spotLight->SetIntensity(10.);
}

void Room::Tick() { this->GameObject3D::Tick(); }

void Room::SetSize(float size) { Room::size = size; }

void Room::SetWallState(Room::WallIndex wallindex, Room::WallState wallState) {

	std::shared_ptr<Wall> wall = this->walls[wallindex].lock();
	wall->SetWallState(wallState);
}

void Room::SetParent(std::weak_ptr<GameObject> parentWeak) {
	if (auto basePtr = parentWeak.lock()) {
		if (auto shipPtr = std::dynamic_pointer_cast<SpaceShip>(basePtr)) {

			this->GameObject3D::SetParent(parentWeak);
		} else {
			Logger::Error("Trying to set non SpaceShip obj as parent to Room");
			return;
		}
	} else {
		Logger::Error("Trying to set parent when parent doesn't exist???");
		return;
	}
}

void Room::SetupPathfindingNodes(std::shared_ptr<SpaceShip> spaceShip, std::shared_ptr<Room> roomPtr) {

	size_t nodeCount = this->pathfindingNodes.size();

	const float nodeHeight = 2.0f;

	// 9 nodes in local room-space, 1 in the center, and 8 around the center in a circle.
	std::array<DirectX::XMVECTOR, 9> nodesLocal = {};
	nodesLocal[0] = DirectX::XMVectorSet(0, nodeHeight, 0, 1);
	float distance = this->size / 4;
	for (size_t i = 1; i < nodeCount; ++i) {
		float angle = DirectX::XM_PIDIV2 - (i - 1) * (DirectX::XM_PI / 4); // Start at North (z+) and go clockwise
		nodesLocal[i] = DirectX::XMVectorSet(std::cos(angle) * distance, nodeHeight, std::sin(angle) * distance, 1);
	}

	// Create vertices for each node using the factory
	for (size_t i = 0; i < nodeCount; ++i) {
		// Create through factory so myPtr is initialized properly
		auto vertexWeak = this->factory->CreateGameObjectOfType<AStarVertex>();
		auto vertex = vertexWeak.lock();

		// Initialize the vertex with its position
		vertex->Initialize(nodesLocal[i]);
		vertex->transform.SetPosition(nodesLocal[i]);

		// Now SetParent will work because myPtr is initialized by the factory
		vertex->SetParent(roomPtr);

		this->pathfindingNodes[i] = vertex;

		spaceShip->GetPathfinder()->AddVertex(vertex);
	}

	// Create edges from all outside nodes to the center node
	unsigned int edgeCost = static_cast<unsigned int>(this->size / 3);
	for (size_t i = 1; i < nodeCount; ++i) {
		spaceShip->GetPathfinder()->AddEdge(this->pathfindingNodes[0], this->pathfindingNodes[i], edgeCost);
	}

	edgeCost = static_cast<unsigned int>(this->size / 4);
	// Create edges between adjacent outside nodes
	for (size_t i = 1; i < nodeCount; ++i) {
		size_t nextIndex = (i % 8) + 1;
		spaceShip->GetPathfinder()->AddEdge(this->pathfindingNodes[i], this->pathfindingNodes[nextIndex], edgeCost);
	}
}

void Room::ShowBuildMenu(std::shared_ptr<Player> player) {
	try {
		if (!this->factory) return;

		// Never open build menu while quit-to-menu prompt is visible
		if (player && player->hud && player->hud->IsQuitPromptVisible()) {
			return;
		}

		// If something is already built here, don't open the build menu
		if (!this->builtObject.expired()) {
			Logger::Log("Room::ShowBuildMenu aborted: already built object present");
			return;
		}

		// If a menu already exists, don't create another
		if (!this->buildMenu.expired()) return;

		std::shared_ptr<UI::CanvasObject> canvasObjShared;
		try {
			auto crossWeak = this->factory->FindObjectOfType<Crosshair>();
			if (!crossWeak.expired()) {
				auto cross = crossWeak.lock();
				if (auto parentWeak = cross->GetParent(); !parentWeak.expired()) {
					if (auto parentShared = parentWeak.lock()) {
						if (auto canvasShared = std::dynamic_pointer_cast<UI::CanvasObject>(parentShared)) {
							canvasObjShared = canvasShared;
						}
					}
				}
			}
		} catch (...) {
		}

		std::shared_ptr<UI::CanvasObject> localCanvasCreated;
		std::shared_ptr<UI::CanvasObject> canvasObj;
		if (!canvasObjShared) {
			auto canvasWeak = this->factory->CreateGameObjectOfType<UI::CanvasObject>();
			if (canvasWeak.expired()) return;
			canvasObj = std::static_pointer_cast<UI::CanvasObject>(canvasWeak.lock());
			canvasObj->SetParent(this->GetPtr());
			localCanvasCreated = canvasObj;
		} else {
			auto localCanvasWeak = this->factory->CreateGameObjectOfType<UI::CanvasObject>();
			if (localCanvasWeak.expired()) return;
			canvasObj = std::static_pointer_cast<UI::CanvasObject>(localCanvasWeak.lock());
			canvasObj->SetParent(canvasObjShared->GetPtr());
			localCanvasCreated = canvasObj;
		}

		// Create three buttons: Turret, Generator, Mine
		struct BtnSpec {
			std::string label;
			std::function<void()> cb;
			UI::Vec2 pos;
		};
		
		// Barebone specs: only labels. Positions will be computed to center the buttons beneath the crosshair.
		std::string turretLabel = std::format("Turret, Cost {}", this->turretCost.getCostString());
		std::string generatorLabel = std::format("Generator, Cost {}", this->generatorCost.getCostString());
		std::string mineLabel = std::format("Mine, Cost {}", this->mineCost.getCostString());

		std::vector<std::string> labels = {turretLabel, generatorLabel, mineLabel};

		

		// Determine canvas size (fallback to camera aspect if unavailable) and crosshair center
		UI::Vec2 canvasSize{0.0f, 0.0f};
		UI::Vec2 crossCenter{canvasSize.x * 0.5f, canvasSize.y * 0.5f};
		try {
			if (auto cvs = canvasObj->GetCanvas(); cvs) {
				canvasSize = cvs->GetSize();
			}
		} catch (...) {
		}
		if (canvasSize.x == 0.0f || canvasSize.y == 0.0f) {
			try {
				auto& cam = CameraObject::GetMainCamera();
				float aspect = cam.GetAspectRatio();
				float height = 1080.0f;
				float width = aspect * height;
				canvasSize.x = width;
				canvasSize.y = height;
			} catch (...) {
				canvasSize.x = 1920.0f;
				canvasSize.y = 1080.0f;
			}
		}

		// If we found a real Crosshair, center under it
		try {
			auto crossWeak = this->factory->FindObjectOfType<Crosshair>();
			if (!crossWeak.expired()) {
				auto cross = crossWeak.lock();
				auto pos = cross->GetPosition();
				auto size = cross->GetSize();
				crossCenter.x = pos.x + size.x * 0.5f;
				crossCenter.y = pos.y + size.y * 0.5f;
			} else {
				crossCenter.x = canvasSize.x * 0.5f;
				crossCenter.y = canvasSize.y * 0.5f;
			}
		} catch (...) {
			crossCenter.x = canvasSize.x * 0.5f;
			crossCenter.y = canvasSize.y * 0.5f;
		}

		// Button layout
		const float btnW = 150.0f;
		const float btnH = 40.0f;
		const float spacing = 20.0f;
		const size_t n = labels.size();
		const float totalW = n * btnW + (n - 1) * spacing;
		const float centerX = crossCenter.x;
		const float centerY = crossCenter.y;
		const float startX = centerX - totalW * 0.5f;
		const float yPos = centerY + (btnH * 0.5f) + 40.0f; // just below crosshair

		for (size_t i = 0; i < labels.size(); ++i) {
			const auto& label = labels[i];
			auto btnWeak = this->factory->CreateGameObjectOfType<UI::Button>();
			if (btnWeak.expired()) continue;
			auto btn = std::dynamic_pointer_cast<UI::Button>(btnWeak.lock());
			btn->SetParent(canvasObj->GetPtr());
			btn->SetName(label);
			btn->SetLabel(label);

			btn->SetHorizontalAlign(UI::Button::HorizontalAlign::CENTER);
			btn->SetVerticalAlign(UI::Button::VerticalAlign::MIDDLE);
			UI::Vec2 pos{startX + static_cast<float>(i) * (btnW + spacing), yPos};
			btn->SetPosition(pos);
			btn->SetSize({150, 40});
			btn->SetVisible(true);
			canvasObj->AddChild(std::static_pointer_cast<UI::Widget>(btn));
			RenderQueue::AddUIWidget(btn);
			// Hide menu and restore player input on click. Spawn directly from Room for each type.
			std::weak_ptr<GameObject> me = this->GetPtr();
			auto playerWeak = std::weak_ptr<Player>(player);
			
			bool isGenerator = (label == generatorLabel);
			bool isTurret = (label == turretLabel);
			bool isMine = (label == mineLabel);

			btn->SetOnClick([me, playerWeak, isGenerator, isTurret, isMine]() {
				if (auto roomPtr = std::dynamic_pointer_cast<Room>(me.lock())) {
					bool built = false;
					if (isGenerator && GameManager::GetInstance()->GetPlayer()->resources.tryToPay(
										   roomPtr->generatorCost.getTitanium(), roomPtr->generatorCost.getLubricant(),
							roomPtr->generatorCost.getCarbonFiber(), roomPtr->generatorCost.getCircuit())) {
						built = roomPtr->TryBuildGenerator();
					} else if (isTurret &&
							   GameManager::GetInstance()->GetPlayer()->resources.tryToPay(
								   roomPtr->turretCost.getTitanium(), roomPtr->turretCost.getLubricant(),
								   roomPtr->turretCost.getCarbonFiber(), roomPtr->turretCost.getCircuit())) {
						built = roomPtr->TryBuildTurret();
					} else if (isMine &&
							   GameManager::GetInstance()->GetPlayer()->resources.tryToPay(
								   roomPtr->mineCost.getTitanium(), roomPtr->mineCost.getLubricant(),
											 roomPtr->mineCost.getCarbonFiber(), roomPtr->mineCost.getCircuit())) {
						built = roomPtr->TryBuildMine();
					}
					if (built) {
						roomPtr->HideBuildMenu();
						if (auto p = playerWeak.lock()) {
							p->SetShowCursor(false);
							p->SetInputEnabled(true);
						}
					} else {
						Logger::Error("Bob the builder overslept");
					}
				}
			});
		}

		// Create a Cancel button beneath the middle button
		try {
			float middleX = startX + static_cast<float>(1) * (btnW + spacing);
			UI::Vec2 cancelPos{middleX, yPos + btnH + spacing};
			auto cancelWeak = this->factory->CreateGameObjectOfType<UI::Button>();
			if (!cancelWeak.expired()) {
				auto cancelBtn = std::dynamic_pointer_cast<UI::Button>(cancelWeak.lock());
				cancelBtn->SetParent(canvasObj->GetPtr());
				cancelBtn->SetName("Cancel");
				cancelBtn->SetLabel("Cancel");
				cancelBtn->SetHorizontalAlign(UI::Button::HorizontalAlign::CENTER);
				cancelBtn->SetVerticalAlign(UI::Button::VerticalAlign::MIDDLE);
				cancelBtn->SetPosition(cancelPos);
				cancelBtn->SetSize({150, 40});
				cancelBtn->SetVisible(true);
				canvasObj->AddChild(std::static_pointer_cast<UI::Widget>(cancelBtn));
				RenderQueue::AddUIWidget(cancelBtn);
				std::weak_ptr<GameObject> me = this->GetPtr();
				auto playerWeak = std::weak_ptr<Player>(player);
				cancelBtn->SetOnClick([me, playerWeak]() {
					if (auto roomPtr = std::dynamic_pointer_cast<Room>(me.lock())) {
						roomPtr->HideBuildMenu();
						if (auto p = playerWeak.lock()) {
							p->SetShowCursor(false);
							p->SetInputEnabled(true);
						}
					}
				});
			}
		} catch (...) {
		}

		// Remember menu, show cursor and disable player input
		this->buildMenu = canvasObj;
		if (player) {
			player->SetShowCursor(true);
			player->SetInputEnabled(false);
		}

	} catch (const std::exception& e) {
		Logger::Error("Room::ShowBuildMenu exception: ", e.what());
	} catch (...) {
		Logger::Error("Room::ShowBuildMenu unknown exception");
	}
}

void Room::HideBuildMenu() {
	if (this->buildMenu.expired()) return;
	auto menu = this->buildMenu.lock();
	// Clear children and queue delete for the canvas object
	menu->Clear();
	this->factory->QueueDeleteGameObject(menu);
	this->buildMenu.reset();
}

void Room::Hover() {
	try {
		float currentTime = Time::GetInstance().GetSessionTime();
		if (currentTime < this->hoverDisabledUntil) return;
		auto promptWeak = this->factory->FindObjectOfType<UI::InteractionPrompt>();
		if (promptWeak.expired()) return;
		auto prompt = promptWeak.lock();
		if (!prompt) return;

		std::string txt = "Press \"F\" to open build menu";
		DirectX::XMVECTOR worldPos = this->transform.GetGlobalPosition();
		prompt->Show(txt, worldPos);
	} catch (const std::exception& e) {
		Logger::Error("Room::Hover exception: ", e.what());
	} catch (...) {
		Logger::Error("Room::Hover unknown exception");
	}
}

bool Room::IsBuildMenuOpen() { return !this->buildMenu.expired(); }

void Room::CloseBuildMenu() { this->HideBuildMenu(); }

bool Room::TryBuildGenerator() {
	Logger::Log("Room::TryBuildGenerator called");
	try {
		// Only build if a menu is open and nothing is already built
		if (this->buildMenu.expired()) {
			Logger::Log("TryBuildGenerator failed: buildMenu expired");
			return false;
		}
		if (!this->factory) {
			Logger::Log("TryBuildGenerator failed: factory is null");
			return false;
		}
		if (!this->builtObject.expired()) {
			Logger::Log("TryBuildGenerator failed: builtObject already exists");
			return false;
		}

		auto slotWeak = this->buildSlot;
		if (slotWeak.expired()) {
			Logger::Log("TryBuildGenerator failed: build slot expired");
			return false;
		}
		auto slot = slotWeak.lock();

		auto gen = this->factory->CreateStaticGameObject<ResourceGenerator>();

		gen->SetParent(this->GetPtr());
		gen->transform.SetPosition(0, 1.5, 0);

		DirectX::XMFLOAT3 shipScale;
		DirectX::XMStoreFloat3(&shipScale, this->transform.GetGlobalScale());
		gen->transform.SetScale(DirectX::XMVECTOR({(1.0f / shipScale.x), (1.0f / shipScale.y), (1.0f / shipScale.z)}));

		this->builtObject = static_pointer_cast<GameObject3D>(gen.Get());

		if (!this->GetParent().expired()) {
			auto spaceship = static_pointer_cast<SpaceShip>(GetParent().lock());
			spaceship->GetPathfinder()->RemoveVertex(this->pathfindingNodes[0]);
		}

		// Disable hover on the build slot now that something is built here
		auto slotForHover = this->buildSlot;
		if (!slotForHover.expired()) {
			auto slotPtr = slotForHover.lock();
			if (slotPtr) {
				slotPtr->SetOnHover([] {});
				slotPtr->SetOnInteract([](std::shared_ptr<Player>) {});
			}
		}
		return true;
	} catch (const std::exception& e) {
		Logger::Error("Room::TryBuildGenerator exception: ", e.what());
	} catch (...) {
		Logger::Error("Room::TryBuildGenerator unknown exception");
	}
	return false;
}

bool Room::TryBuildTurret() {
	Logger::Log("Room::TryBuildTurret called");
	try {
		if (this->buildMenu.expired()) {
			Logger::Log("TryBuildTurret failed: buildMenu expired");
			return false;
		}
		if (!this->factory) {
			Logger::Log("TryBuildTurret failed: factory is null");
			return false;
		}
		if (!this->builtObject.expired()) {
			Logger::Log("TryBuildTurret failed: builtObject already exists");
			return false;
		}

		auto slotWeak = this->buildSlot;
		if (slotWeak.expired()) {
			Logger::Log("TryBuildTurret failed: build slot expired");
			return false;
		}
		auto slot = slotWeak.lock();

		auto turret = this->factory->CreateStaticGameObject<Turret>();

		turret->SetParent(this->GetPtr());
		turret->transform.SetPosition(0, 0.6, 0);

		DirectX::XMFLOAT3 shipScale;
		DirectX::XMStoreFloat3(&shipScale, this->transform.GetGlobalScale());
		Logger::Log(shipScale.x, ", ", shipScale.y, ", ", shipScale.z);
		turret->transform.SetScale(
			DirectX::XMVECTOR({(1.0f / shipScale.x), (1.0f / shipScale.y), (1.0f / shipScale.z)}));

		this->builtObject = static_pointer_cast<GameObject3D>(turret.Get());

		if (!this->GetParent().expired()) {
			auto spaceship = static_pointer_cast<SpaceShip>(GetParent().lock());
			spaceship->GetPathfinder()->RemoveVertex(this->pathfindingNodes[0]);
		}

		// Disable hover on the build slot now that something is built here
		auto slotForHover = this->buildSlot;
		if (!slotForHover.expired()) {
			auto slotPtr = slotForHover.lock();
			if (slotPtr) {
				slotPtr->SetOnHover([] {});
				slotPtr->SetOnInteract([](std::shared_ptr<Player>) {});
			}
		}
		return true;
	} catch (const std::exception& e) {
		Logger::Error("Room::TryBuildTurret exception: ", e.what());
	} catch (...) {
		Logger::Error("Room::TryBuildTurret unknown exception");
	}
	return false;
}

bool Room::TryBuildMine() {
	Logger::Log("Room::TryBuildMine called");
	try {
		if (this->buildMenu.expired()) {
			Logger::Log("TryBuildMine failed: buildMenu expired");
			return false;
		}
		if (!this->factory) {
			Logger::Log("TryBuildMine failed: factory is null");
			return false;
		}
		if (!this->builtObject.expired()) {
			Logger::Log("TryBuildMine failed: builtObject already exists");
			return false;
		}

		auto slotWeak = this->buildSlot;
		if (slotWeak.expired()) {
			Logger::Log("TryBuildMine failed: build slot expired");
			return false;
		}
		auto slot = slotWeak.lock();

		auto mine = this->factory->CreateStaticGameObject<Mine>();

		mine->SetParent(this->GetPtr());
		mine->transform.SetPosition(slot->transform.GetPosition());
		mine->SetName("Mine");

		DirectX::XMFLOAT3 shipScale;
		DirectX::XMStoreFloat3(&shipScale, this->transform.GetGlobalScale());
		mine->transform.SetScale(DirectX::XMVECTOR({(1.0f / shipScale.x), (1.0f / shipScale.y), (1.0f / shipScale.z)}));

		// Make the interact collider work again after the mine explodes
		mine->SetPostExplosion([&] {
			if (this->buildSlot.expired()) return;

			auto build = this->buildSlot.lock();
			build->SetOnInteract([&](std::shared_ptr<Player> p) {
				if (this->builtObject.expired() && !GameManager::GetInstance()->GetInCombat()) {
					this->ShowBuildMenu(p);
				}
			});
			build->SetOnHover([&] { this->Hover(); });
		});

		this->builtObject = static_pointer_cast<GameObject3D>(mine.Get());
		// Disable hover on the build slot now that something is built here
		auto& slotForHover = this->buildSlot;
		if (!slotForHover.expired()) {
			auto slotPtr = slotForHover.lock();
			if (slotPtr) {
				slotPtr->SetOnHover([] {});
				slotPtr->SetOnInteract([](std::shared_ptr<Player>) {});
			}
		}
		return true;
	} catch (const std::exception& e) {
		Logger::Error("Room::TryBuildMine exception: ", e.what());
	} catch (...) {
		Logger::Error("Room::TryBuildMine unknown exception");
	}
	return false;
}