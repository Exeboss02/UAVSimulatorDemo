#include "rendering/renderer.h"
#include "UI/button.h"
#include "UI/image.h"
#include "UI/text.h"
#include "UI/textRenderer.h"
#include "UI/widget.h"
#include "core/filepathHolder.h"
#include "gameObjects/objectLoader.h"
#include <chrono>
#include <cmath>
#include <memory>
#include "gameObjects/SpaceShipObj.h"

#define DEBUG_TIMER

Renderer::Renderer()
	: viewport(), currentPixelShader(nullptr), currentVertexShader(nullptr), currentRasterizerState(nullptr),
	  currentMaterial(nullptr), maximumSpotlights(16),
	  staticObjectsTree({-SpaceShip::ROOM_SIZE, -20, -SpaceShip::ROOM_SIZE},
						{SpaceShip::ROOM_SIZE * (SpaceShip::SHIP_MAX_SIZE_X + 1), 20,
						 SpaceShip::ROOM_SIZE * (SpaceShip::SHIP_MAX_SIZE_Y + 1)},
						5, 4),
	  renderQueue(this->meshRenderQueue, this->spotLightRenderQueue, this->pointLightRenderQueue,
				  this->staticObjectsTree, this->uiRenderQueue) {
	this->renderQueue.newSkyboxCallback = [this](std::string filename) { this->ChangeSkybox(filename); };
}

void Renderer::Init(const Window& window) {
	SetViewport(window);

	CreateDeviceAndSwapChain(window);
	CreateRenderTarget();
	CreateDepthBuffer(window);

	// Initialize FW1FontWrapper for text rendering
	UI::TextRenderer::GetInstance().InitializeFW1(this->device.Get());

	this->spotLightShadows.Init(this->device.Get(), 256, this->maximumSpotlights);

	this->spotLightViewPort = {
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<FLOAT>(256),
		.Height = static_cast<FLOAT>(256),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};

	this->pointLightShadows.Init(this->device.Get(), 256, this->maximumSpotlights);

	this->pointLightViewPort = this->spotLightViewPort;
}

void Renderer::SetAllDefaults() {
	CreateSampler();

	CreateInputLayout();

	CreateRasterizerStates();

	CreateRendererConstantBuffers();

	LoadShaders();

	this->skybox = std::make_unique<Skybox>();

	this->skybox->Init(this->device.Get(), this->immediateContext.Get(),
					   (FilepathHolder::GetAssetsDirectory() / "skybox" / "bright_asteroid.dds").string());

	// FW1FontWrapper handles font loading at runtime; no atlas preload needed
}

std::vector<std::weak_ptr<MeshObject>> Renderer::GetVisibleObjects(CameraObject& camera) {
	std::vector<std::weak_ptr<MeshObject>> visible = this->staticObjectsTree.GetVisibleElements(camera);

	visible.reserve(this->meshRenderQueue.size());

	DirectX::XMVECTOR cameraGlobalPos = camera.transform.GetGlobalPosition();

	for (size_t i = 0; i < this->meshRenderQueue.size(); i++) {
		if (this->meshRenderQueue[i].expired()) {
			// This should remove deleted lights
			Logger::Log("The renderer deleted an object");
			this->meshRenderQueue.erase(this->meshRenderQueue.begin() + i);
			i--;
			continue;
		}

		float farPlane = camera.GetFarPlane();
		if (farPlane < 100) {
			float distance = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(
				cameraGlobalPos, this->meshRenderQueue[i].lock()->transform.GetGlobalPosition())));

			if (distance > std::powf(camera.GetFarPlane(), 2.0f) + 9) {
				continue;
			}
		}

		visible.emplace_back(this->meshRenderQueue[i]);
	}

	return visible;
}

void Renderer::SetViewport(const Window& window) {
	RECT rc{};
	GetClientRect(window.GetHWND(), &rc);
	UINT clientWidth = static_cast<UINT>(rc.right - rc.left);
	UINT clientHeight = static_cast<UINT>(rc.bottom - rc.top);
	if (clientWidth == 0 || clientHeight == 0) {
		clientWidth = window.GetWidth();
		clientHeight = window.GetHeight();
	}
	if (clientWidth == 0 || clientHeight == 0) {
		clientWidth = 1;
		clientHeight = 1;
	}

	this->viewport.Width = static_cast<FLOAT>(clientWidth);
	this->viewport.Height = static_cast<FLOAT>(clientHeight);
	this->viewport.MinDepth = 0.0f;
	this->viewport.MaxDepth = 1.0f;
	this->viewport.TopLeftX = 0;
	this->viewport.TopLeftY = 0;
}

void Renderer::CreateDeviceAndSwapChain(const Window& window) {
	RECT rc{};
	GetClientRect(window.GetHWND(), &rc);
	UINT clientWidth = static_cast<UINT>(rc.right - rc.left);
	UINT clientHeight = static_cast<UINT>(rc.bottom - rc.top);
	if (clientWidth == 0 || clientHeight == 0) {
		clientWidth = window.GetWidth();
		clientHeight = window.GetHeight();
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = clientWidth;
	swapChainDesc.BufferDesc.Height = clientHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_UNORDERED_ACCESS | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window.GetHWND();
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	HRESULT hr =
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0,
									  D3D11_SDK_VERSION, &swapChainDesc, this->swapChain.GetAddressOf(),
									  this->device.GetAddressOf(), nullptr, this->immediateContext.GetAddressOf());

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create device and swapchain, Error: " + hr);
	}
}

void Renderer::CreateRenderTarget() {
	this->renderTarget = std::unique_ptr<RenderTarget>(new RenderTarget());
	this->renderTarget->Init(this->device.Get(), this->swapChain.Get());
}

void Renderer::CreateDepthBuffer(const Window& window) {
	RECT rc{};
	GetClientRect(window.GetHWND(), &rc);
	UINT clientWidth = static_cast<UINT>(rc.right - rc.left);
	UINT clientHeight = static_cast<UINT>(rc.bottom - rc.top);

	if (clientWidth == 0 || clientHeight == 0) {
		clientWidth = window.GetWidth();
		clientHeight = window.GetHeight();
	}

	if (clientWidth == 0 || clientHeight == 0) {
		DXGI_SWAP_CHAIN_DESC desc{};
		if (SUCCEEDED(this->swapChain->GetDesc(&desc))) {
			clientWidth = desc.BufferDesc.Width;
			clientHeight = desc.BufferDesc.Height;
		}
	}

	if (clientWidth == 0 || clientHeight == 0) {
		clientWidth = 1;
		clientHeight = 1;
	}

	this->depthBuffer = std::unique_ptr<DepthBuffer>(new DepthBuffer());
	this->depthBuffer->Init(this->device.Get(), clientWidth, clientHeight);
}

void Renderer::CreateInputLayout() {
	// Input layout for non-instanced drawing

	this->inputLayout = std::unique_ptr<InputLayout>(new InputLayout());
	this->inputLayout->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	this->inputLayout->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	this->inputLayout->AddInputElement("UV", DXGI_FORMAT_R32G32_FLOAT);

	auto vsShader1 = AssetManager::GetInstance().GetShaderPtr("VSSkybox")->GetShaderByteCode();
	this->inputLayout->FinalizeInputLayout(this->device.Get(), vsShader1.c_str(), vsShader1.length());

	// Input layout for instanced drawing

	this->instanceInputLayout = std::make_unique<InputLayout>();
	this->instanceInputLayout->PrepareInputLayout(3 + 8);

	// Standard stuff
	this->instanceInputLayout->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	this->instanceInputLayout->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	this->instanceInputLayout->AddInputElement("UV", DXGI_FORMAT_R32G32_FLOAT);

	// World matrix
	this->instanceInputLayout->AddInputElement("WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);
	this->instanceInputLayout->AddInputElement("WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);
	this->instanceInputLayout->AddInputElement("WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);
	this->instanceInputLayout->AddInputElement("WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);

	// Inversed transposed matrix for normals
	this->instanceInputLayout->AddInputElement("INVERSED_TRANSPOSED_WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);
	this->instanceInputLayout->AddInputElement("INVERSED_TRANSPOSED_WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);
	this->instanceInputLayout->AddInputElement("INVERSED_TRANSPOSED_WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);
	this->instanceInputLayout->AddInputElement("INVERSED_TRANSPOSED_WORLD_MATRIX", DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
											   D3D11_INPUT_PER_INSTANCE_DATA);

	auto vsShader2 = AssetManager::GetInstance().GetShaderPtr("VSStandard")->GetShaderByteCode();
	this->instanceInputLayout->FinalizeInputLayout(this->device.Get(), vsShader2.c_str(), vsShader2.length());
}

void Renderer::CreateSampler() {
	this->sampler = std::unique_ptr<Sampler>(new Sampler());
	this->sampler->Init(this->device.Get(), D3D11_TEXTURE_ADDRESS_WRAP);

	this->shadowSampler = std::unique_ptr<Sampler>(new Sampler());
	this->shadowSampler->Init(this->device.Get(), D3D11_TEXTURE_ADDRESS_BORDER, D3D11_FILTER_ANISOTROPIC,
							  D3D11_COMPARISON_LESS_EQUAL, {1, 1, 1, 1});

	// Create a dedicated UI/font sampler using point filtering and clamp to avoid blurring and edge bleeding
	this->uiSampler = std::make_unique<Sampler>();
	this->uiSampler->Init(this->device.Get(), D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER_MIN_MAG_MIP_POINT);

	// Linear UI sampler for smooth upscaling when requested
	this->uiLinearSampler = std::make_unique<Sampler>();
	this->uiLinearSampler->Init(this->device.Get(), D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	// Create an alpha blend state for UI (font) rendering
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Microsoft::WRL::ComPtr<ID3D11BlendState> bs;
	HRESULT hr = this->device->CreateBlendState(&blendDesc, bs.GetAddressOf());
	if (SUCCEEDED(hr)) {
		this->alphaBlendState = bs;
	} else {
		Logger::Log("Failed to create alpha blend state for UI text");
	}
}

void Renderer::CreateRasterizerStates() {
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(rastDesc));
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.DepthClipEnable = TRUE;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	this->standardRasterizerState = std::make_unique<RasterizerState>();
	this->standardRasterizerState->Init(this->device.Get(), &rastDesc);

	// Wireframe rasterizer desc doesn't cull anything and draws in wireframe
	D3D11_RASTERIZER_DESC wireframeRastDesc;
	ZeroMemory(&wireframeRastDesc, sizeof(wireframeRastDesc));
	wireframeRastDesc.CullMode = D3D11_CULL_NONE;
	wireframeRastDesc.DepthClipEnable = TRUE;
	wireframeRastDesc.FillMode = D3D11_FILL_WIREFRAME; // Wireframe
	this->wireframeRasterizerState = std::make_unique<RasterizerState>();
	this->wireframeRasterizerState->Init(this->device.Get(), &wireframeRastDesc);

	// UI rasterizer: no culling, solid fill
	D3D11_RASTERIZER_DESC uiRastDesc;
	ZeroMemory(&uiRastDesc, sizeof(uiRastDesc));
	uiRastDesc.CullMode = D3D11_CULL_NONE;
	uiRastDesc.DepthClipEnable = TRUE;
	uiRastDesc.FillMode = D3D11_FILL_SOLID;
	this->uiRasterizerState = std::make_unique<RasterizerState>();
	this->uiRasterizerState->Init(this->device.Get(), &uiRastDesc);

	// This ended up being the same as the normal rasterizerDesc,
	// but I'm leaving it in case there was something wrong with skybox
	// because I feel like it should be CULL_FRONT and not CULL_BACK
	D3D11_RASTERIZER_DESC skyboxRastDesc;
	ZeroMemory(&skyboxRastDesc, sizeof(skyboxRastDesc));
	skyboxRastDesc.CullMode = D3D11_CULL_BACK;
	skyboxRastDesc.DepthClipEnable = TRUE;
	skyboxRastDesc.FillMode = D3D11_FILL_SOLID;
	this->skyboxRasterizerState = std::make_unique<RasterizerState>();
	this->skyboxRasterizerState->Init(this->device.Get(), &skyboxRastDesc);
}

void Renderer::CreateRenderMap(RenderMap& renderMap, CameraObject& camera) {
#ifdef DEBUG_TIMER
	const auto start{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	//// Removes dead gameobjects
	// this->meshRenderQueue.erase(std::remove_if(this->meshRenderQueue.begin(), this->meshRenderQueue.end(),
	//									 [](const std::weak_ptr<MeshObject>& w) { return w.expired(); }),
	//							this->meshRenderQueue.end());

	auto renderQueue = GetVisibleObjects(camera);

	renderMap.meshes.clear();

	for (size_t i = 0; i < renderQueue.size(); i++) {
		std::shared_ptr<MeshObject> meshObject = renderQueue[i].lock();

		if (!meshObject->IsActive() || meshObject->IsHidden()) continue;

		auto& meshObjData = meshObject->GetMesh();

		std::string meshIdentifier = meshObjData.GetMeshIdentifier();
		auto [meshIterator, meshInserted] = renderMap.meshes.try_emplace(meshIdentifier);

		auto& mapMesh = meshIterator->second;

		if (meshInserted) {
			// Create mesh entry
			mapMesh.mesh = meshObjData.GetMesh().lock();
			mapMesh.submeshes = std::vector<RenderMap::RenderMapSubmesh>(mapMesh.mesh->GetSubMeshes().size());
		}

		// Create the world matrices
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT4X4 worldMatrixInverseTransposed;

		DirectX::XMStoreFloat4x4(&worldMatrix, meshObject->transform.GetGlobalWorldMatrix(false));
		DirectX::XMStoreFloat4x4(&worldMatrixInverseTransposed, meshObject->transform.GetGlobalWorldMatrix(true));

		RenderMap::WorldMatrixBufferContainer worldMatrixBufferContainer = {worldMatrix, worldMatrixInverseTransposed};

		for (size_t j = 0; j < mapMesh.submeshes.size(); j++) {
			auto material = meshObjData.GetMaterial(j).lock();
			std::string materialIdentifier = material->GetIdentifier();
			auto [materialIterator, materialInserted] = mapMesh.submeshes[j].materials.try_emplace(materialIdentifier);

			auto& mapMaterial = materialIterator->second;

			if (materialInserted) {
				// Create new material
				mapMaterial.material = material;
			}

			mapMaterial.objects.push_back(worldMatrixBufferContainer);
		}
	}

#ifdef DEBUG_TIMER
	const auto finsihedRenderMap{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds{finsihedRenderMap - start};
	ImGui::Text(
		("Render map creation: " + std::to_string(elapsedSeconds.count()) + " : " + std::to_string(renderQueue.size()))
			.c_str());
#endif // DEBUG_TIMER
}

void Renderer::CreateCheapRenderMap(CheapRenderMap& renderMap, CameraObject& camera) {
#ifdef DEBUG_TIMER
	const auto start{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	renderMap.meshes.clear();

	if (this->renderAllWireframe) return;

	auto renderQueue = GetVisibleObjects(camera);

	for (size_t i = 0; i < renderQueue.size(); i++) {
		std::shared_ptr<MeshObject> meshObject = renderQueue[i].lock();

		if (!meshObject->IsActive() || meshObject->IsHidden()) continue;

		auto& meshObjData = meshObject->GetMesh();

		if (meshObjData.GetMaterial(0).lock()->wireframe) continue;

		std::string meshIdentifier = meshObjData.GetMeshIdentifier();
		auto [meshIterator, meshInserted] = renderMap.meshes.try_emplace(meshIdentifier);

		auto& mapMesh = meshIterator->second;

		if (meshInserted) {
			// Create mesh entry
			mapMesh.mesh = meshObjData.GetMesh().lock();
		}

		// Create the world matrices
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT4X4 worldMatrixInverseTransposed;

		DirectX::XMStoreFloat4x4(&worldMatrix, meshObject->transform.GetGlobalWorldMatrix(false));
		DirectX::XMStoreFloat4x4(&worldMatrixInverseTransposed, meshObject->transform.GetGlobalWorldMatrix(true));

		RenderMap::WorldMatrixBufferContainer worldMatrixBufferContainer = {worldMatrix, worldMatrixInverseTransposed};

		mapMesh.objects.push_back(worldMatrixBufferContainer);
	}

#ifdef DEBUG_TIMER
	const auto finsihedRenderMap{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds{finsihedRenderMap - start};
	ImGui::Text(
		("Cheap Render map: " + std::to_string(elapsedSeconds.count()) + " : " + std::to_string(renderQueue.size()))
			.c_str());
#endif // DEBUG_TIMER
}

size_t Renderer::FillRenderMap(RenderMap& renderMap, CameraObject& camera) {
#ifdef DEBUG_TIMER
	const auto start{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	auto renderQueue = GetVisibleObjects(camera);

	if (renderQueue.size() <= 0) {
		return 0;
	}

	for (auto& mesh : renderMap.meshes) {
		for (auto& submesh : mesh.second.submeshes) {
			for (auto& material : submesh.materials) {
				material.second.objects.clear();
			}
		}
	}

	for (auto& meshObjectWeak : renderQueue) {
		std::shared_ptr<MeshObject> meshObject = meshObjectWeak.lock();

		if (!meshObject->IsActive() || meshObject->IsHidden()) continue;

		auto& meshObjData = meshObject->GetMesh();

		std::string meshIdentifier = meshObjData.GetMeshIdentifier();

		auto& mapMesh = renderMap.meshes[meshIdentifier];

		// Create the world matrices
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT4X4 worldMatrixInverseTransposed;

		DirectX::XMStoreFloat4x4(&worldMatrix, meshObject->transform.GetGlobalWorldMatrix(false));
		DirectX::XMStoreFloat4x4(&worldMatrixInverseTransposed, meshObject->transform.GetGlobalWorldMatrix(true));

		RenderMap::WorldMatrixBufferContainer worldMatrixBufferContainer = {worldMatrix, worldMatrixInverseTransposed};

		for (size_t j = 0; j < mapMesh.submeshes.size(); j++) {
			auto material = meshObjData.GetMaterial(j).lock();
			std::string materialIdentifier = material->GetIdentifier();

			auto& mapMaterial = mapMesh.submeshes[j].materials[materialIdentifier];

			mapMaterial.objects.push_back(worldMatrixBufferContainer);
		}
	}

#ifdef DEBUG_TIMER
	const auto finsihedRenderMap{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds{finsihedRenderMap - start};
	ImGui::Text(
		("Render map fill: " + std::to_string(elapsedSeconds.count()) + " : " + std::to_string(renderQueue.size()))
			.c_str());
#endif // DEBUG_TIMER

	return renderQueue.size();
}

void Renderer::CreateRendererConstantBuffers() {
	CameraObject::CameraMatrixContainer camMatrix = {};
	this->cameraBuffer = std::make_unique<ConstantBuffer>();
	this->cameraBuffer->Init(this->device.Get(), sizeof(CameraObject::CameraMatrixContainer), &camMatrix,
							 D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	// Renderer::WorldMatrixBufferContainer worldMatrix = {};
	// this->worldMatrixBuffer = std::make_unique<ConstantBuffer>();
	// this->worldMatrixBuffer->Init(this->device.Get(), sizeof(Renderer::WorldMatrixBufferContainer), &worldMatrix,
	//							  D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	this->spotlightBuffer = std::make_unique<StructuredBuffer<SpotlightObject::SpotLightContainer>>();
	this->spotlightBuffer->Init(this->device.Get(), this->maximumSpotlights, {});

	this->pointlightBuffer = std::make_unique<StructuredBuffer<PointLightObject::PointLightContainer>>();
	this->pointlightBuffer->Init(this->device.Get(), this->maximumSpotlights, {});

	Renderer::LightCountBufferContainer lightCountContainer = {};
	this->spotlightCountBuffer = std::make_unique<ConstantBuffer>();
	this->spotlightCountBuffer->Init(this->device.Get(), sizeof(Renderer::LightCountBufferContainer),
									 &lightCountContainer, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	this->pointlightCountBuffer = std::make_unique<ConstantBuffer>();
	this->pointlightCountBuffer->Init(this->device.Get(), sizeof(Renderer::LightCountBufferContainer),
									  &lightCountContainer, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void Renderer::LoadShaders() {
	this->vertexShader = AssetManager::GetInstance().GetShaderPtr("VSStandard");
	this->pixelShaderLit = AssetManager::GetInstance().GetShaderPtr("PSLit");
	this->pixelShaderUnlit = AssetManager::GetInstance().GetShaderPtr("PSUnlit");

	this->defaultMat = AssetManager::GetInstance().GetMaterialWeakPtr("defaultLitMaterial");
	this->defaultUnlitMat = AssetManager::GetInstance().GetMaterialWeakPtr("defaultUnlitMaterial");
}

void Renderer::Render() {
#ifdef DEBUG_TIMER
	ImGui::Begin("Timer");
	const auto start{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	CreateRenderMap(this->standardRenderMap, CameraObject::GetMainCamera());

#ifdef DEBUG_TIMER
	const auto beforeShadow{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	BindInputLayout(this->instanceInputLayout.get());
	this->ShadowPass();
	ID3D11ShaderResourceView* spotLightShaderResourceView = this->spotLightShadows.GetShaderResourceView();
	ID3D11ShaderResourceView* pointLightShaderResourceView = this->pointLightShadows.GetSrv();
	this->GetContext()->PSSetShaderResources(5, 1, &spotLightShaderResourceView);
	this->GetContext()->PSSetShaderResources(7, 1, &pointLightShaderResourceView);

#ifdef DEBUG_TIMER
	const auto afterShadow{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds{afterShadow - beforeShadow};
	ImGui::Text(("Shadow pass: " + std::to_string(elapsed_seconds.count())).c_str());
#endif // DEBUG_TIMER

	RenderPass();

#ifdef DEBUG_TIMER
	const auto afterRenderPass{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds2{afterRenderPass - afterShadow};
	ImGui::Text(("Entire renderpass function: " + std::to_string(elapsed_seconds2.count())).c_str());
#endif // DEBUG_TIMER

	// Unbinding shadowmaps to allow input on them again
	ID3D11ShaderResourceView* nullview = nullptr;
	this->GetContext()->PSSetShaderResources(5, 1, &nullview);

	// Unbinding shadowmaps to allow input on them again
	this->GetContext()->PSSetShaderResources(7, 1, &nullview);

#ifdef DEBUG_TIMER
	const auto afterRender{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds3{afterRender - start};
	ImGui::Text(("Entire Render function: " + std::to_string(elapsed_seconds3.count())).c_str());
	ImGui::End();
#endif // DEBUG_TIMER
}

void Renderer::Present() { this->swapChain->Present(this->isVSyncEnabled ? 1 : 0, 0); }

void Renderer::Resize(const Window& window) {
	this->ResizeSwapChain(window);

	BindRenderTarget();
	BindViewport();
}

void Renderer::ToggleVSync(bool enable) { this->isVSyncEnabled = enable; }

void Renderer::ToggleWireframe(bool enable) {
	Logger::Log("test");
	this->renderAllWireframe = enable;
}

void Renderer::ChangeSkybox(std::string filepath) {
	this->skybox->SwapCubemap(this->device.Get(), this->immediateContext.Get(), filepath);
}

ID3D11Device* Renderer::GetDevice() const { return this->device.Get(); }

ID3D11DeviceContext* Renderer::GetContext() const { return this->immediateContext.Get(); }

IDXGISwapChain* Renderer::GetSwapChain() const { return this->swapChain.Get(); }

void Renderer::RenderPass() {

#ifdef DEBUG_TIMER
	const auto start{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	// Bind basic stuff (it probably doesn't need to be set every frame)
	if (!this->hasBoundStatic) {
		BindSampler();
		BindRenderTarget();
		BindViewport();

		this->hasBoundStatic = true;
	}

	// Clear last frame
	ClearRenderTargetViewAndDepthStencilView();

	// Bind frame specific stuff
	BindCameraMatrix();
	BindLights();

	// Bind skybox
	DrawSkybox();

	// Fix input layout after skybox changed it
	BindInputLayout(this->instanceInputLayout.get());

	// Bind rasterizerState
	if (!this->renderAllWireframe) {
		BindRasterizerState(this->standardRasterizerState.get());
	} else {
		BindMaterial(this->defaultUnlitMat.lock().get());
		BindRasterizerState(this->wireframeRasterizerState.get());
	}

#ifdef DEBUG_TIMER
	const auto afterBinds{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds{afterBinds - start};
	ImGui::Text(("Render pass setup: " + std::to_string(elapsed_seconds.count())).c_str());

	const auto startColorPass{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	RenderRenderMap(this->standardRenderMap);

#ifdef DEBUG_TIMER
	const auto endColorPass{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds{endColorPass - startColorPass};
	ImGui::Text(("Color pass: " + std::to_string(elapsedSeconds.count())).c_str());
#endif // DEBUG_TIMER

	// Render UI in a separate pass
	RenderUI();
}

void Renderer::RenderUI() {
#ifdef DEBUG_TIMER
	const auto startUIPass{std::chrono::steady_clock::now()};
#endif // DEBUG_TIMER

	// Prepare an orthographic camera matching the render target (top-left origin)
	CameraObject::CameraMatrixContainer uiCamera{};
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX proj =
		DirectX::XMMatrixOrthographicOffCenterLH(0.0f, this->viewport.Width, this->viewport.Height, 0.0f, 0.0f, 1.0f);
	DirectX::XMMATRIX viewProj = view * proj;
	DirectX::XMStoreFloat4x4(&uiCamera.viewProjectionMatrix, viewProj);
	uiCamera.cameraPosition = DirectX::XMVectorZero();

	// Update camera buffer for UI
	this->cameraBuffer->UpdateBuffer(this->immediateContext.Get(), &uiCamera);
	ID3D11Buffer* camBuf = this->cameraBuffer->GetBuffer();
	this->immediateContext->VSSetConstantBuffers(0, 1, &camBuf);

	// Disable depth testing so UI draws on top
	this->immediateContext->OMSetDepthStencilState(nullptr, 0);

	// Build a list of live widgets, remove expired entries and sort by zIndex
	std::vector<std::shared_ptr<UI::Widget>> widgets;
	for (size_t i = 0; i < this->uiRenderQueue.size(); i++) {
		if (this->uiRenderQueue[i].expired()) {
			this->uiRenderQueue.erase(this->uiRenderQueue.begin() + i);
			i--;
			continue;
		}
		auto w = this->uiRenderQueue[i].lock();
		if (!w) continue;
		if (!w->IsVisible() || !w->isEnabled()) continue;
		widgets.push_back(w);
	}

	std::sort(widgets.begin(), widgets.end(),
			  [](const std::shared_ptr<UI::Widget>& a, const std::shared_ptr<UI::Widget>& b) {
				  int za = a->GetZIndex();
				  int zb = b->GetZIndex();
				  if (za != zb) return za < zb;

				  return std::uintptr_t(a.get()) < std::uintptr_t(b.get());
			  });

	for (auto& widget : widgets) {
		widget->Draw();

		if (!widget->GetMesh().GetMesh().expired()) {
			this->RenderMeshObject(widget.get(), true);
		}

		if (auto img = dynamic_cast<UI::Image*>(widget.get())) {
			const std::string ident = img->GetImage();
			if (!ident.empty()) {
				// Ensure texture is loaded (AssetManager::GetTexture may attempt to load it)
				AssetManager::GetInstance().GetTexture(ident);
				auto texWeak = AssetManager::GetInstance().GetTextureWeakPtr(ident);
				if (!texWeak.expired()) {
					auto tex = texWeak.lock();
					ID3D11ShaderResourceView* srv = tex->GetSrv();
					// no logging here to avoid spamming per-frame

					// Build quad vertices matching widget position/size (top-left origin)
					std::vector<Vertex> vertices;
					std::vector<uint32_t> indices;
					float x0 = img->GetPosition().x;
					float y0 = img->GetPosition().y;
					float x1 = x0 + img->GetSize().x;
					float y1 = y0 + img->GetSize().y;

					// Use widget global Z so image quads layer consistently with mesh widgets
					DirectX::XMVECTOR globalPos = img->transform.GetGlobalPosition();
					float widgetZ = globalPos.m128_f32[2];

					Vertex vTL{}; // top-left
					vTL.pos[0] = x0;
					vTL.pos[1] = y0;
					vTL.pos[2] = widgetZ;
					vTL.normal[0] = vTL.normal[1] = vTL.normal[2] = 0.0f;
					vTL.uv[0] = 0.0f;
					vTL.uv[1] = 1.0f - 0.0f;

					Vertex vTR{}; // top-right
					vTR.pos[0] = x1;
					vTR.pos[1] = y0;
					vTR.pos[2] = widgetZ;
					vTR.normal[0] = vTR.normal[1] = vTR.normal[2] = 0.0f;
					vTR.uv[0] = 1.0f;
					vTR.uv[1] = 1.0f - 0.0f;

					Vertex vBL{}; // bottom-left
					vBL.pos[0] = x0;
					vBL.pos[1] = y1;
					vBL.pos[2] = widgetZ;
					vBL.normal[0] = vBL.normal[1] = vBL.normal[2] = 0.0f;
					vBL.uv[0] = 0.0f;
					vBL.uv[1] = 1.0f - 1.0f;

					Vertex vBR{}; // bottom-right
					vBR.pos[0] = x1;
					vBR.pos[1] = y1;
					vBR.pos[2] = widgetZ;
					vBR.normal[0] = vBR.normal[1] = vBR.normal[2] = 0.0f;
					vBR.uv[0] = 1.0f;
					vBR.uv[1] = 1.0f - 1.0f;

					vertices.push_back(vTL);
					vertices.push_back(vTR);
					vertices.push_back(vBL);
					vertices.push_back(vBR);

					indices.push_back(0);
					indices.push_back(2);
					indices.push_back(3);

					indices.push_back(0);
					indices.push_back(3);
					indices.push_back(1);

					// Use image tint
					DirectX::XMFLOAT4 color = img->GetTint();

					// Ensure UI blend and rasterizer states are set for proper UI drawing
					float blendFactor[4] = {0, 0, 0, 0};
					this->immediateContext->OMSetBlendState(this->alphaBlendState.Get(), blendFactor, 0xffffffff);
					this->BindRasterizerState(this->uiRasterizerState.get());

					this->DrawTextQuads(vertices, indices, srv, color, false);
				}
			}
		}

		// No per-widget text flush; text will be rendered once after the UI pass
	}

	BindRenderTarget();

	float blendFactor[4] = {0, 0, 0, 0};
	this->immediateContext->OMSetBlendState(this->alphaBlendState.Get(), blendFactor, 0xffffffff);
	this->BindRasterizerState(this->uiRasterizerState.get());

	// Ensure depth test is disabled for UI text (BindRenderTarget rebinds depth state)
	this->immediateContext->OMSetDepthStencilState(nullptr, 0);
	// Render all text submissions now (TextRenderer sorts by zIndex)
	UI::TextRenderer::GetInstance().Render(this->immediateContext.Get());
	// Restore default rasterizer and blend state
	this->immediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	this->BindRasterizerState(this->standardRasterizerState.get());

#ifdef DEBUG_TIMER
	const auto endUIPass{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds2{endUIPass - startUIPass};
	ImGui::Text(("UI pass: " + std::to_string(elapsedSeconds2.count())).c_str());
#endif // DEBUG_TIMER
}

void Renderer::ShadowPass() {

	this->hasBoundStatic = false;
	if (this->currentVertexShader != this->vertexShader.get()) {
		this->vertexShader->BindShader(this->immediateContext.Get());
		this->currentVertexShader = this->vertexShader.get();
	}
	this->GetContext()->PSSetShader(nullptr, nullptr, 0);

	this->SpotLightShadowPass();
	this->PointLightShadowPass();

	// Rebind default pixel shader
	this->pixelShaderLit->BindShader(this->immediateContext.Get());
	this->currentPixelShader = this->pixelShaderLit.get();

	// Reset renderTarget and deapthStencil
	this->BindRenderTarget();

	// Reset ViewPort
	this->BindViewport();
}

void Renderer::SpotLightShadowPass() {

	const uint32_t lightCount = std::min<uint32_t>(this->spotLightRenderQueue.size(), this->maximumSpotlights);

	for (uint32_t i = 0; i < lightCount; i++) {
		if ((this->spotLightRenderQueue)[i].expired()) {
			// This should remove deleted lights
			Logger::Log("The renderer deleted a light");
			this->spotLightRenderQueue.erase(this->spotLightRenderQueue.begin() + i);
			i--;
			continue;
		}

		auto light = (this->spotLightRenderQueue)[i].lock();

		this->immediateContext->ClearDepthStencilView(this->spotLightShadows.GetDepthStencilView(i), D3D11_CLEAR_DEPTH,
													  1, 0);
		this->immediateContext->OMSetRenderTargets(0, nullptr, this->spotLightShadows.GetDepthStencilView(i));

		if (light->camera.expired()) {
			Logger::Error("Lights shadow camera was dead");
			continue;
		}
		auto& camera = *light->camera.lock().get();
		auto matrixContainer = camera.GetCameraMatrix();

		this->immediateContext->RSSetViewports(1, &this->spotLightViewPort);
		this->cameraBuffer->UpdateBuffer(this->GetContext(), &matrixContainer);

		ID3D11Buffer* buffer = this->cameraBuffer->GetBuffer();
		this->immediateContext->VSSetConstantBuffers(0, 1, &buffer);

		// Draw all objects to depthstencil
		CheapRenderMap thisCameraRenderMap;
		this->CreateCheapRenderMap(thisCameraRenderMap, camera);
		this->RenderCheapRenderMap(thisCameraRenderMap);
	}
}

void Renderer::PointLightShadowPass() {
	uint32_t lightCount = std::min<uint32_t>(this->pointLightRenderQueue.size(), this->maximumSpotlights);

	for (uint32_t i = 0; i < lightCount; i++) {
		if (this->pointLightRenderQueue[i].expired()) {
			// This should remove deleted lights
			Logger::Log("The renderer deleted a light");
			this->pointLightRenderQueue.erase(this->pointLightRenderQueue.begin() + i);
			i--;
			lightCount = std::min<uint32_t>(this->pointLightRenderQueue.size(), this->maximumSpotlights);
			continue;
		}

		auto light = this->pointLightRenderQueue[i].lock();

		for (size_t j = 0; j < 6; j++) {

			this->immediateContext->ClearDepthStencilView(this->pointLightShadows.GetDsv(i, j), D3D11_CLEAR_DEPTH, 1,
														  0);
			this->immediateContext->OMSetRenderTargets(0, nullptr, this->pointLightShadows.GetDsv(i, j));

			if (light->cameras[j].expired()) {
				Logger::Error("Lights shadow camera was dead");
				continue;
			}
			auto& camera = *light->cameras[j].lock().get();
			auto matrixContainer = camera.GetCameraMatrix();

			this->immediateContext->RSSetViewports(1, &this->pointLightViewPort);
			this->cameraBuffer->UpdateBuffer(this->GetContext(), &matrixContainer);

			ID3D11Buffer* buffer = this->cameraBuffer->GetBuffer();
			this->immediateContext->VSSetConstantBuffers(0, 1, &buffer);

			// Draw all objects to depthstencil
			CheapRenderMap thisCameraRenderMap;
			this->CreateCheapRenderMap(thisCameraRenderMap, camera);
			this->RenderCheapRenderMap(thisCameraRenderMap);
		}
	}
}

void Renderer::ClearRenderTargetViewAndDepthStencilView() {
	// Clear previous frame
	float clearColor[4] = {0, 0, 0.1, 0};
	this->immediateContext->ClearRenderTargetView(this->renderTarget->GetRenderTargetView(), clearColor);
	this->immediateContext->ClearDepthStencilView(this->depthBuffer->GetDepthStencilView(0),
												  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void Renderer::ResizeSwapChain(const Window& window) {
	if (window.GetWidth() == 0 || window.GetHeight() == 0) {
		return;
	}

	// Unbind any views using the swapchain
	if (this->immediateContext) {
		this->immediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

	// Release old views
	this->renderTarget.reset();
	this->depthBuffer.reset();

	// Resize swapchain
	HRESULT hr = this->swapChain->ResizeBuffers(0, window.GetWidth(), window.GetHeight(), DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to resize swapchain buffers, Error: " + hr);
	}

	// Recreate views
	CreateRenderTarget();
	CreateDepthBuffer(window);

	// Update viewport
	SetViewport(window);
}

void Renderer::BindSampler() {
	// Sampler
	ID3D11SamplerState* sampler = this->sampler->GetSamplerState();
	immediateContext->PSSetSamplers(0, 1, &sampler);

	// Shadow sampler
	ID3D11SamplerState* shadowSampler = this->shadowSampler->GetSamplerState();
	immediateContext->PSSetSamplers(1, 1, &shadowSampler);
}

void Renderer::BindInputLayout(InputLayout* inputLayout) {
	// Set up inputs
	this->immediateContext->IASetInputLayout(inputLayout->GetInputLayout());
	this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::BindRenderTarget() {
	// Render target
	ID3D11RenderTargetView* rtv = this->renderTarget->GetRenderTargetView();
	this->immediateContext->OMSetRenderTargets(1, &rtv, this->depthBuffer->GetDepthStencilView(0));
	this->immediateContext->OMSetDepthStencilState(this->depthBuffer->GetDepthStencilState(), 1);
}

void Renderer::BindViewport() { this->immediateContext->RSSetViewports(1, &this->viewport); }

void Renderer::BindRasterizerState(RasterizerState* rastState) {
	if (this->currentRasterizerState == rastState) return;

	if (rastState == nullptr) {
		Logger::Error("RasterizerState is nullptr");
	}

	this->immediateContext->RSSetState(rastState->GetRasterizerState());

	this->currentRasterizerState = rastState;
}

void Renderer::BindMaterial(BaseMaterial* material) {
	if (this->currentMaterial == material) return;

	RenderData renderData = material->GetRenderData(this->immediateContext.Get());

	if (material->wireframe) {
		if (this->currentRasterizerState != this->wireframeRasterizerState.get()) {
			BindRasterizerState(this->wireframeRasterizerState.get());
		}
	} else {
		if (this->currentRasterizerState == this->wireframeRasterizerState.get()) {
			BindRasterizerState(this->standardRasterizerState.get());
		}
	}

	// Bind shaders
	// Checks to avoid making unnecessary GPU calls
	// Since shaders will almost always be the same
	if (renderData.vertexShader) {
		if (this->currentVertexShader != renderData.vertexShader.get()) {
			renderData.vertexShader->BindShader(this->immediateContext.Get());
			this->currentVertexShader = renderData.vertexShader.get();
		}
	} else {
		if (this->currentVertexShader != this->vertexShader.get()) {
			this->vertexShader->BindShader(this->immediateContext.Get());
			this->currentVertexShader = this->vertexShader.get();
		}
	}

	if (renderData.pixelShader) {
		if (this->currentPixelShader != renderData.pixelShader.get()) {
			renderData.pixelShader->BindShader(this->immediateContext.Get());
			this->currentPixelShader = renderData.pixelShader.get();
		}
	} else {
		if (this->currentPixelShader != this->pixelShaderLit.get()) {
			this->pixelShaderLit->BindShader(this->immediateContext.Get());
			this->currentPixelShader = this->pixelShaderLit.get();
		}
	}

	// Bind textures: only bind contiguous runs of non-null SRVs to avoid binding nulls
	const UINT baseSlot = 1; // materials use pixel shader slots starting at 1
	if (!renderData.textures.empty()) {
		size_t start = 0;
		while (start < renderData.textures.size()) {
			// skip nulls
			while (start < renderData.textures.size() && renderData.textures[start] == nullptr)
				start++;
			if (start >= renderData.textures.size()) break;

			// find end of contiguous non-null run
			size_t end = start;
			while (end < renderData.textures.size() && renderData.textures[end] != nullptr)
				end++;

			UINT slot = static_cast<UINT>(baseSlot + start);
			UINT count = static_cast<UINT>(end - start);
			this->immediateContext->PSSetShaderResources(slot, count, &renderData.textures[start]);

			start = end;
		}
	}

	// Also bind constant buffers
	for (size_t i = 0; i < renderData.pixelBuffers.size(); i++) {
		ID3D11Buffer* buf = renderData.pixelBuffers[i]->GetBuffer();
		this->immediateContext->PSSetConstantBuffers(i + 1, 1, &buf); // i + 1 because first slot is always occupied
	}

	for (size_t i = 0; i < renderData.vertexBuffers.size(); i++) {
		ID3D11Buffer* buf = renderData.vertexBuffers[i]->GetBuffer();
		this->immediateContext->VSSetConstantBuffers(i + 1, 1,
													 &buf); // i + 1 because first slot is always occupied
	}

	this->currentMaterial = material;
}

void Renderer::BindLights() {
	{
		if (this->spotLightRenderQueue.size() > this->maximumSpotlights) {
			Logger::Warn("Just letting you know, there's more spotlights in the scene than the renderer supports. "
						 "Increase maximumSpotlights.");
		}

		uint32_t lightCount = std::min<uint32_t>(this->spotLightRenderQueue.size(), this->maximumSpotlights);

		if (lightCount > 0) {

			// Inefficient, should be fixed
			std::vector<SpotlightObject::SpotLightContainer> spotlights;
			for (uint32_t i = 0; i < lightCount; i++) {
				if ((this->spotLightRenderQueue)[i].expired()) {
					// This should remove deleted lights
					Logger::Log("The renderer deleted a light");
					this->spotLightRenderQueue.erase(this->spotLightRenderQueue.begin() + i);
					i--;
					// Lazy solution
					BindLights();
					return;
				}

				spotlights.push_back((this->spotLightRenderQueue)[i].lock()->GetSpotLightData());
			}

			// Updates and binds buffer
			this->spotlightBuffer->UpdateBuffer(this->immediateContext.Get(), spotlights);
			ID3D11ShaderResourceView* lightSrv = this->spotlightBuffer->GetSRV();
			this->immediateContext->PSSetShaderResources(0, 1, &lightSrv);
		}

		// Updates and binds light count constant buffer
		Renderer::LightCountBufferContainer lightCountContainer = {lightCount, 1, 1, 1};
		this->spotlightCountBuffer->UpdateBuffer(this->immediateContext.Get(), &lightCountContainer);
		ID3D11Buffer* buf = this->spotlightCountBuffer->GetBuffer();
		this->immediateContext->PSSetConstantBuffers(0, 1, &buf);
	}
	{

		if (this->pointLightRenderQueue.size() > this->maximumSpotlights) {
			Logger::Warn(
				"Just letting you know, there's more pointlights in the scene than the renderer supports. Increase "
				"maximumSpotlights.");
		}

		uint32_t lightCount = std::min<uint32_t>(this->pointLightRenderQueue.size(), this->maximumSpotlights);

		if (lightCount > 0) {

			// Inefficient, should be fixed
			std::vector<PointLightObject::PointLightContainer> pointLights;
			for (uint32_t i = 0; i < lightCount; i++) {
				if ((this->pointLightRenderQueue)[i].expired()) {
					// This should remove deleted lights
					Logger::Log("The renderer deleted a light");
					this->pointLightRenderQueue.erase(this->pointLightRenderQueue.begin() + i);
					i--;
					// Lazy solution
					BindLights();
					return;
				}

				pointLights.push_back((this->pointLightRenderQueue)[i].lock()->GetPointLightData());
			}

			// Updates and binds buffer
			this->pointlightBuffer->UpdateBuffer(this->immediateContext.Get(), pointLights);
			ID3D11ShaderResourceView* lightSrv = this->pointlightBuffer->GetSRV();
			this->immediateContext->PSSetShaderResources(6, 1, &lightSrv);
		}

		// Updates and binds light count constant buffer
		Renderer::LightCountBufferContainer lightCountContainer = {lightCount, 1, 1, 1};
		this->pointlightCountBuffer->UpdateBuffer(this->immediateContext.Get(), &lightCountContainer);
		ID3D11Buffer* buf = this->pointlightCountBuffer->GetBuffer();
		this->immediateContext->PSSetConstantBuffers(2, 1, &buf);
	}
}

void Renderer::BindCameraMatrix() {
	auto cameraMatrix = CameraObject::GetMainCamera().GetCameraMatrix();

	this->cameraBuffer->UpdateBuffer(this->immediateContext.Get(), &cameraMatrix);

	ID3D11Buffer* buffer = this->cameraBuffer->GetBuffer();
	this->immediateContext->VSSetConstantBuffers(0, 1, &buffer);
}

void Renderer::BindWorldMatrix(ID3D11Buffer* buffer) { this->immediateContext->VSSetConstantBuffers(1, 1, &buffer); }

void Renderer::DrawSkybox() {
	BindInputLayout(this->inputLayout.get());
	BindRasterizerState(this->skyboxRasterizerState.get());

	this->skybox->Draw(this->immediateContext.Get());

	// Since the skybox sets shaders
	this->currentVertexShader = nullptr;
	this->currentPixelShader = nullptr;

	// Make sure that a real material is bound
	BindMaterial(this->defaultMat.lock().get());
}

void Renderer::RenderMeshObject(MeshObject* meshObject, bool renderMaterial) {
	// Bind mesh
	MeshObjData data = meshObject->GetMesh();
	std::weak_ptr<Mesh> weak_mesh = data.GetMesh();
	if (weak_mesh.expired()) {
		Logger::Error("Trying to render with expired mesh");
		return;
	}

	std::shared_ptr<Mesh> mesh = weak_mesh.lock();

	VertexBuffer vBuf = mesh->GetVertexBuffer();

	this->immediateContext->IASetIndexBuffer(mesh->GetIndexBuffer().GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	// this->currentMesh = mesh.get();

	// Bind worldmatrix
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, meshObject->transform.GetGlobalWorldMatrix(false));
	DirectX::XMFLOAT4X4 worldMatrixInverseTransposed;
	DirectX::XMStoreFloat4x4(&worldMatrixInverseTransposed, meshObject->transform.GetGlobalWorldMatrix(true));

	RenderMap::WorldMatrixBufferContainer worldMatrixBufferContainer = {worldMatrix, worldMatrixInverseTransposed};

	size_t instanceCount(1);
	InstanceBuffer* instanceBuffer = GetInstanceBuffer(instanceCount, &worldMatrixBufferContainer);

	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	strides[0] = vBuf.GetVertexSize();
	strides[1] = instanceBuffer->GetInstanceSize();

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointers[0] = vBuf.GetBuffer();
	bufferPointers[1] = instanceBuffer->GetBuffer();

	this->immediateContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

	// this->worldMatrixBuffer->UpdateBuffer(this->immediateContext.Get(), &worldMatrixBufferContainer);
	// BindWorldMatrix(this->worldMatrixBuffer->GetBuffer());

	// Draw submeshes
	size_t index = 0;
	for (auto& subMesh : mesh->GetSubMeshes()) {
		std::weak_ptr<BaseMaterial> weak_material = data.GetMaterial(index);

		if (weak_material.expired()) {
			Logger::Error("Trying to render expired material, trying to continue...");
		} else {
			if (!this->renderAllWireframe && renderMaterial) {
				std::shared_ptr<BaseMaterial> sharedMaterial = weak_material.lock();
				if (sharedMaterial.get() != this->currentMaterial) {
					BindMaterial(sharedMaterial.get());
				}
			}

			// Draw to screen
			this->immediateContext->DrawIndexedInstanced(subMesh.GetNrOfIndices(), 1, subMesh.GetStartIndex(), 0, 0);
		}

		index++;
	}
}

void Renderer::DrawTextQuads(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
							 ID3D11ShaderResourceView* srv, const DirectX::XMFLOAT4& color, bool useLinearFilter) {
	if (vertices.empty() || indices.empty()) {
		Logger::Warn("Renderer::DrawTextQuads: no vertices or no indices");
		return;
	}

	// Create transient vertex and index buffers
	VertexBuffer vbuf;
	vbuf.Init(this->device.Get(), sizeof(Vertex), static_cast<UINT>(vertices.size()), (void*) vertices.data());

	IndexBuffer ibuf;
	ibuf.Init(this->device.Get(), indices.size(), (uint32_t*) indices.data());

	// Bind buffers
	this->immediateContext->IASetIndexBuffer(ibuf.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

	// Set world matrix identity
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMFLOAT4X4 worldMatrixInvTrans;
	DirectX::XMStoreFloat4x4(&worldMatrixInvTrans, DirectX::XMMatrixIdentity());
	RenderMap::WorldMatrixBufferContainer wm{worldMatrix, worldMatrixInvTrans};

	size_t instanceCount(1);
	InstanceBuffer* instanceBuffer = GetInstanceBuffer(instanceCount, &wm);

	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	strides[0] = vbuf.GetVertexSize();
	strides[1] = instanceBuffer->GetInstanceSize();

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointers[0] = vbuf.GetBuffer();
	bufferPointers[1] = instanceBuffer->GetBuffer();

	this->immediateContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

	// Create a temporary unlit material (heap-allocated to avoid pointer-collision
	// with renderer's material caching) using the provided SRV and apply tint color
	auto tempMat = std::make_unique<UnlitMaterial>(this->device.Get());
	tempMat->unlitShader = AssetManager::GetInstance().GetShaderPtr("PSUnlit");
	tempMat->diffuseTexture = std::make_shared<Texture>(srv, "__ui_temp");
	tempMat->color[0] = color.x;
	tempMat->color[1] = color.y;
	tempMat->color[2] = color.z;
	tempMat->color[3] = color.w;
	// Honor global wireframe toggle so UI quads reflect renderAllWireframe
	tempMat->wireframe = this->renderAllWireframe;

	// Use dedicated UI sampler for text rendering. Choose linear when requested (scaling up), otherwise point.
	ID3D11SamplerState* fontSampler = nullptr;
	if (useLinearFilter) {
		if (this->uiLinearSampler) fontSampler = this->uiLinearSampler->GetSamplerState();
	} else {
		if (this->uiSampler) fontSampler = this->uiSampler->GetSamplerState();
	}
	// Save current sampler at slot 0
	ID3D11SamplerState* prevSampler = nullptr;
	this->immediateContext->PSGetSamplers(0, 1, &prevSampler);
	if (fontSampler) this->immediateContext->PSSetSamplers(0, 1, &fontSampler);

	// Bind material and draw
	BindMaterial(tempMat.get());
	this->immediateContext->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);
	// Prevent renderer from caching pointer to this ephemeral material
	this->currentMaterial = nullptr;

	// Restore previous sampler
	if (prevSampler) this->immediateContext->PSSetSamplers(0, 1, &prevSampler);
	if (prevSampler) prevSampler->Release();
}

void Renderer::RenderRenderMap(RenderMap& renderMap, bool renderMaterials) {
	for (auto& [meshName, mesh] : renderMap.meshes) {
		// The indexbuffer is the same no matter which submesh or material
		this->immediateContext->IASetIndexBuffer(mesh.mesh->GetIndexBuffer().GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

		for (size_t i = 0; i < mesh.submeshes.size(); i++) {
			auto& submesh = mesh.submeshes[i];
			auto& submeshData = mesh.mesh->GetSubMeshes();

			for (auto& [materialName, material] : submesh.materials) {

				size_t instanceCount = material.objects.size();
				if (instanceCount <= 0) continue;

				// Only set material if it's needed
				if (!this->renderAllWireframe && renderMaterials) {
					BindMaterial(material.material.get());
				}

				// Get the instance buffer
				InstanceBuffer* newInstanceBuffer = GetInstanceBuffer(instanceCount, material.objects.data());

				// Set vertex buffers

				VertexBuffer vBuf = mesh.mesh->GetVertexBuffer();

				unsigned int strides[2];
				unsigned int offsets[2];
				ID3D11Buffer* bufferPointers[2];

				strides[0] = vBuf.GetVertexSize();
				strides[1] = newInstanceBuffer->GetInstanceSize();

				offsets[0] = 0;
				offsets[1] = 0;

				bufferPointers[0] = vBuf.GetBuffer();
				bufferPointers[1] = newInstanceBuffer->GetBuffer();

				this->immediateContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

				// Draw call
				this->immediateContext->DrawIndexedInstanced(submeshData[i].GetNrOfIndices(),
															 newInstanceBuffer->GetNrOfInstances(),
															 submeshData[i].GetStartIndex(), 0, 0);
			}
		}
	}
}

void Renderer::RenderCheapRenderMap(CheapRenderMap& renderMap) {
	for (auto& [meshName, mesh] : renderMap.meshes) {
		this->immediateContext->IASetIndexBuffer(mesh.mesh->GetIndexBuffer().GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

		size_t instanceCount = mesh.objects.size();
		if (instanceCount <= 0) continue;

		// Get the instance buffer
		InstanceBuffer* newInstanceBuffer = GetInstanceBuffer(instanceCount, mesh.objects.data());

		// Set vertex buffers

		VertexBuffer vBuf = mesh.mesh->GetVertexBuffer();

		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];

		strides[0] = vBuf.GetVertexSize();
		strides[1] = newInstanceBuffer->GetInstanceSize();

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = vBuf.GetBuffer();
		bufferPointers[1] = newInstanceBuffer->GetBuffer();

		this->immediateContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

		// Submesh Test
		for (auto& subMesh : mesh.mesh->GetSubMeshes()) {
			// Draw to screen
			this->immediateContext->DrawIndexedInstanced(subMesh.GetNrOfIndices(), instanceCount,
														 subMesh.GetStartIndex(), 0, 0);
		}

		// Draw call
		// this->immediateContext->DrawInstanced(vBuf.GetNrOfVertices(), newInstanceBuffer->GetNrOfInstances(), 0, 0);
	}
}

InstanceBuffer* Renderer::GetInstanceBuffer(size_t& instanceCount, void* data) {
	InstanceBuffer* newInstanceBuffer = nullptr;
	if (this->instanceBuffers.contains(instanceCount)) {
		newInstanceBuffer = this->instanceBuffers[instanceCount].get();
		newInstanceBuffer->Update(this->immediateContext.Get(), sizeof(RenderMap::WorldMatrixBufferContainer),
								  instanceCount, data);
	} else {
		this->instanceBuffers.emplace(instanceCount, std::make_unique<InstanceBuffer>());
		newInstanceBuffer = this->instanceBuffers[instanceCount].get();
		newInstanceBuffer->Init(this->device.Get(), sizeof(RenderMap::WorldMatrixBufferContainer), instanceCount, data);

		if (this->instanceBuffers.size() > 100) {
			Logger::Warn("A few too many instance buffers!!");
		}
	}
	return newInstanceBuffer;
}
