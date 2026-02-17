#include "rendering/renderer.h"
#include "UI/text.h"
#include "UI/textRenderer.h"
#include "UI/widget.h"
#include "core/filepathHolder.h"
#include "gameObjects/objectLoader.h"
#include <chrono>

Renderer::Renderer()
	: viewport(), currentPixelShader(nullptr), currentVertexShader(nullptr), currentRasterizerState(nullptr),
	  currentMaterial(nullptr), maximumSpotlights(16),
	  renderQueue(this->meshRenderQueue, this->spotLightRenderQueue, this->pointLightRenderQueue, this->uiRenderQueue) 
{
	this->renderQueue.newSkyboxCallback = [this](std::string filename) { this->ChangeSkybox(filename); };
}

void Renderer::Init(const Window& window) {
	SetViewport(window);

	CreateDeviceAndSwapChain(window);
	CreateRenderTarget();
	CreateDepthBuffer(window);
}

void Renderer::SetAllDefaults() {
	CreateSampler();

	CreateInputLayout();

	CreateRasterizerStates();

	CreateRendererConstantBuffers();

	LoadShaders();

	this->skybox = std::make_unique<Skybox>();

	this->skybox->Init(this->device.Get(), this->immediateContext.Get(),
					   (FilepathHolder::GetAssetsDirectory() / "skybox" / "asteroids.dds").string());

	// Preload default UI font atlas so TextRenderer can render immediately
	UI::TextRenderer::GetInstance().LoadFont("default", this->device.Get());
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
	this->inputLayout->FinalizeInputLayout(
		this->device.Get(), vsShader1.c_str(), vsShader1.length());


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

void Renderer::CreateRenderMap() {
	const auto start{std::chrono::steady_clock::now()};

	// Removes dead gameobjects
	this->meshRenderQueue.erase(std::remove_if(this->meshRenderQueue.begin(), this->meshRenderQueue.end(),
										 [](const std::weak_ptr<MeshObject>& w) { return w.expired(); }),
								this->meshRenderQueue.end());

	this->standardRenderMap.meshes.clear();
	
	for (size_t i = 0; i < this->meshRenderQueue.size(); i++) {
		std::shared_ptr<MeshObject> meshObject = this->meshRenderQueue[i].lock();

		if (!meshObject->IsActive() || meshObject->IsHidden()) continue;

		auto& meshObjData = meshObject->GetMesh();

		std::string meshIdentifier = meshObjData.GetMeshIdentifier();
		auto [meshIterator, meshInserted] = this->standardRenderMap.meshes.try_emplace(meshIdentifier);

		auto& mapMesh = meshIterator->second;

		if (meshInserted) {
			// Create mesh entry
			mapMesh.mesh = meshObjData.GetMesh().lock();
			mapMesh.submeshes = std::vector<RenderMap::RenderMapSubmesh>(mapMesh.mesh->GetSubMeshes().size());
		}


		// Create the world matrices
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT4X4 worldMatrixInverseTransposed;

		DirectX::XMStoreFloat4x4(&worldMatrix, meshObject->GetGlobalWorldMatrix(false));
		DirectX::XMStoreFloat4x4(&worldMatrixInverseTransposed, meshObject->GetGlobalWorldMatrix(true));

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

	const auto finsihedRenderMap{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds{finsihedRenderMap - start};
	ImGui::Text(("Render map creation: " + std::to_string(elapsedSeconds.count())).c_str());
}

void Renderer::CreateRendererConstantBuffers() {
	CameraObject::CameraMatrixContainer camMatrix = {};
	this->cameraBuffer = std::make_unique<ConstantBuffer>();
	this->cameraBuffer->Init(this->device.Get(), sizeof(CameraObject::CameraMatrixContainer), &camMatrix,
							 D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	//Renderer::WorldMatrixBufferContainer worldMatrix = {};
	//this->worldMatrixBuffer = std::make_unique<ConstantBuffer>();
	//this->worldMatrixBuffer->Init(this->device.Get(), sizeof(Renderer::WorldMatrixBufferContainer), &worldMatrix,
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
	ImGui::Begin("Timer");

	const auto start{std::chrono::steady_clock::now()};

	CreateRenderMap();

	BindInputLayout(this->instanceInputLayout.get());
	auto shadowmaps = this->ShadowPass();
	this->GetContext()->PSSetShaderResources(5, shadowmaps.spotlightSRVs.size(), shadowmaps.spotlightSRVs.data());
	this->GetContext()->PSSetShaderResources(7, shadowmaps.pointLightSRVs.size(), shadowmaps.pointLightSRVs.data());

	const auto afterShadow{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds{afterShadow - start};
	ImGui::Text(("Shadow pass: " + std::to_string(elapsed_seconds.count())).c_str());


	RenderPass();
	const auto afterRender{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds2{afterRender - afterShadow};
	ImGui::Text(("Entire renderpass: " + std::to_string(elapsed_seconds2.count())).c_str());

	// Unbinding shadowmaps to allow input on them again
	for (auto& spotLightShadowMaps : shadowmaps.spotlightSRVs) {
		// Since shadowmaps vector is just a non owning clone of all views, it is safe to just set them to nullptr
		// for unbinding the shadowmaps
		spotLightShadowMaps = nullptr;
	}
	this->GetContext()->PSSetShaderResources(5, shadowmaps.spotlightSRVs.size(), shadowmaps.spotlightSRVs.data());

	// Unbinding shadowmaps to allow input on them again
	for (auto& pointLightShadowMaps : shadowmaps.pointLightSRVs) {
		// Since shadowmaps vector is just a non owning clone of all views, it is safe to just set them to nullptr
		// for unbinding the shadowmaps
		pointLightShadowMaps = nullptr;
	}
	this->GetContext()->PSSetShaderResources(7, shadowmaps.pointLightSRVs.size(), shadowmaps.pointLightSRVs.data());

	ImGui::End();
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
	const auto start{std::chrono::steady_clock::now()};

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

	// World Matrix Buffer only needs to be bound once per frame
	//BindWorldMatrix(this->worldMatrixBuffer->GetBuffer());

	const auto afterBinds{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds{afterBinds - start};
	ImGui::Text(("Render pass setup: " + std::to_string(elapsed_seconds.count())).c_str());

	const auto startColorPass{std::chrono::steady_clock::now()};

	RenderRenderMap(this->standardRenderMap);

	const auto endColorPass{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsedSeconds{endColorPass - startColorPass};
	ImGui::Text(("Color pass: " + std::to_string(elapsedSeconds.count())).c_str());

	// UI pass: render UI widgets in an orthographic projection on top of the scene

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

	// Render only UI widgets from the dedicated UI render queue
	for (size_t i = 0; i < this->uiRenderQueue.size(); i++) {
		auto widgetWeak = this->uiRenderQueue[i];

		if (widgetWeak.expired()) {
			this->uiRenderQueue.erase(this->uiRenderQueue.begin() + i);
			i--;
			continue;
		}

		auto widget = widgetWeak.lock();
		if (!widget->IsVisible() || !widget->isEnabled()) continue;

		widget->Draw();

		if (!widget->GetMesh().GetMesh().expired()) {
			this->RenderMeshObject(widget.get(), true);
		}
	}

	// Restore depth/stencil by rebinding render target (rebinds depth stencil)
	BindRenderTarget();

	// Render submitted text via TextRenderer
	// Enable alpha blending and UI rasterizer for text rendering
	float blendFactor[4] = {0, 0, 0, 0};
	this->immediateContext->OMSetBlendState(this->alphaBlendState.Get(), blendFactor, 0xffffffff);
	this->BindRasterizerState(this->uiRasterizerState.get());

	UI::TextRenderer::GetInstance().Render(this);

	// Restore default rasterizer and blend state
	this->immediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	this->BindRasterizerState(this->standardRasterizerState.get());
}

Renderer::ShadowResourceViews Renderer::ShadowPass() {

	ShadowResourceViews shadowSRVs{};

	this->hasBoundStatic = false;
	if (this->currentVertexShader != this->vertexShader.get()) {
		this->vertexShader->BindShader(this->immediateContext.Get());
		this->currentVertexShader = this->vertexShader.get();
	}
	this->GetContext()->PSSetShader(nullptr, nullptr, 0);

	shadowSRVs.spotlightSRVs = this->SpotLightShadowPass();
	shadowSRVs.pointLightSRVs = this->PointLightShadowPass();

	// Rebind default pixel shader
	this->pixelShaderLit->BindShader(this->immediateContext.Get());
	this->currentPixelShader = this->pixelShaderLit.get();

	// Reset renderTarget and deapthStencil
	this->BindRenderTarget();

	// Reset ViewPort
	this->BindViewport();

	return shadowSRVs;
}

std::vector<ID3D11ShaderResourceView*> Renderer::SpotLightShadowPass() {

	const uint32_t lightCount = std::min<uint32_t>(this->spotLightRenderQueue.size(), this->maximumSpotlights);

	std::vector<ID3D11ShaderResourceView*> depthStencilViews;
	depthStencilViews.reserve(lightCount);

	for (uint32_t i = 0; i < lightCount; i++) {
		if ((this->spotLightRenderQueue)[i].expired()) {
			// This should remove deleted lights
			Logger::Log("The renderer deleted a light");
			this->spotLightRenderQueue.erase(this->spotLightRenderQueue.begin() + i);
			i--;
			continue;
		}

		auto light = (this->spotLightRenderQueue)[i].lock();
		if (light->GetResolutionChanged()) {
			light->SetDepthBuffer(this->GetDevice());
		}

		this->immediateContext->ClearDepthStencilView(light->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1, 0);
		this->immediateContext->OMSetRenderTargets(0, nullptr, light->GetDepthStencilView());

		if (light->camera.expired()) {
			Logger::Error("Lights shadow camera was dead");
			continue;
		}
		auto matrixContainer = light->camera.lock()->GetCameraMatrix();

		const auto& viewPort = light->GetViewPort();
		this->immediateContext->RSSetViewports(1, &viewPort);
		this->cameraBuffer->UpdateBuffer(this->GetContext(), &matrixContainer);

		ID3D11Buffer* buffer = this->cameraBuffer->GetBuffer();
		this->immediateContext->VSSetConstantBuffers(0, 1, &buffer);

		// Draw all objects to depthstencil
		this->RenderRenderMap(this->standardRenderMap, false);
		depthStencilViews.push_back(light->GetSRV());
	}
	return depthStencilViews;
}

std::vector<ID3D11ShaderResourceView*> Renderer::PointLightShadowPass() {
	uint32_t lightCount = std::min<uint32_t>(this->pointLightRenderQueue.size(), this->maximumSpotlights);

	std::vector<ID3D11ShaderResourceView*> depthStencilViews;
	depthStencilViews.reserve(lightCount);

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
		if (light->GetResolutionChanged()) {
			light->SetDepthBuffers(this->GetDevice());
		}

		auto DSViews = light->GetDepthStencilViews();
		auto srv = light->GetSRV();

		for (size_t j = 0; j < 6; j++) {

			this->immediateContext->ClearDepthStencilView(DSViews[j], D3D11_CLEAR_DEPTH, 1, 0);
			this->immediateContext->OMSetRenderTargets(0, nullptr, DSViews[j]);

			if (light->cameras[j].expired()) {
				Logger::Error("Lights shadow camera was dead");
				continue;
			}
			auto matrixContainer = light->cameras[j].lock()->GetCameraMatrix();

			const auto& viewPort = light->GetViewPort();
			this->immediateContext->RSSetViewports(1, &viewPort);
			this->cameraBuffer->UpdateBuffer(this->GetContext(), &matrixContainer);

			ID3D11Buffer* buffer = this->cameraBuffer->GetBuffer();
			this->immediateContext->VSSetConstantBuffers(0, 1, &buffer);

			// Draw all objects to depthstencil
			this->RenderRenderMap(this->standardRenderMap, false);
		}
		depthStencilViews.push_back(srv);
	}

	return depthStencilViews;
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
	if (rastState == nullptr) {
		Logger::Error("RasterizerState is nullptr");
	}

	this->immediateContext->RSSetState(rastState->GetRasterizerState());

	this->currentRasterizerState = rastState;
}

void Renderer::BindMaterial(BaseMaterial* material) {
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

	// Bind textures
	this->immediateContext->PSSetShaderResources(1, renderData.textures.size(), renderData.textures.data());

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
	DirectX::XMStoreFloat4x4(&worldMatrix, meshObject->GetGlobalWorldMatrix(false));
	DirectX::XMFLOAT4X4 worldMatrixInverseTransposed;
	DirectX::XMStoreFloat4x4(&worldMatrixInverseTransposed, meshObject->GetGlobalWorldMatrix(true));

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

	//this->worldMatrixBuffer->UpdateBuffer(this->immediateContext.Get(), &worldMatrixBufferContainer);
	//BindWorldMatrix(this->worldMatrixBuffer->GetBuffer());

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
					this->currentMaterial = sharedMaterial.get();
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


	// Create a temporary unlit material using the provided SRV and apply tint color
	UnlitMaterial tempMat(this->device.Get());
	tempMat.unlitShader = AssetManager::GetInstance().GetShaderPtr("PSUnlit");
	tempMat.diffuseTexture = std::make_shared<Texture>(srv, "__font_atlas");
	tempMat.color[0] = color.x;
	tempMat.color[1] = color.y;
	tempMat.color[2] = color.z;
	tempMat.color[3] = color.w;

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
	BindMaterial(&tempMat);
	this->immediateContext->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);

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
	
				// Only set material if it's needed
				if (!this->renderAllWireframe && renderMaterials) {
					if (material.material.get() != this->currentMaterial) {
						BindMaterial(material.material.get());
						this->currentMaterial = material.material.get();
					}
				}


				// Get the instance buffer
				size_t instanceCount = material.objects.size(); 
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
				this->immediateContext->DrawIndexedInstanced(
					submeshData[i].GetNrOfIndices(), newInstanceBuffer->GetNrOfInstances(), submeshData[i].GetStartIndex(), 0, 0);
			}
		}

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
