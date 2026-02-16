#include "rendering/renderer.h"
#include "core/filepathHolder.h"
#include "gameObjects/objectLoader.h"
#include <chrono>

Renderer::Renderer()
	: viewport(), currentPixelShader(nullptr), currentVertexShader(nullptr), currentRasterizerState(nullptr),
	  currentMaterial(nullptr), currentMesh(nullptr), maximumSpotlights(16),
	  renderQueue(this->meshRenderQueue, this->spotLightRenderQueue, this->pointLightRenderQueue)
{
	this->renderQueue.newSkyboxCallback = [this](std::string filename) { this->ChangeSkybox(filename); };
}

void Renderer::Init(const Window& window) {
	SetViewport(window);

	CreateDeviceAndSwapChain(window);
	CreateRenderTarget();
	CreateDepthBuffer(window);

	CreateRenderQueue();
}

void Renderer::SetAllDefaults() {
	CreateSampler();

	CreateInputLayout(AssetManager::GetInstance().GetShaderPtr("VSStandard")->GetShaderByteCode());

	CreateRasterizerStates();

	CreateRendererConstantBuffers();

	LoadShaders();

	this->skybox = std::make_unique<Skybox>();
	this->skybox->Init(this->device.Get(), this->immediateContext.Get(), (FilepathHolder::GetAssetsDirectory() / "skybox" / "asteroids.dds").string());
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

void Renderer::CreateInputLayout(const std::string& vShaderByteCode) {
	this->inputLayout = std::unique_ptr<InputLayout>(new InputLayout());
	this->inputLayout->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	this->inputLayout->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	this->inputLayout->AddInputElement("UV", DXGI_FORMAT_R32G32_FLOAT);
	this->inputLayout->FinalizeInputLayout(this->device.Get(), vShaderByteCode.c_str(), vShaderByteCode.length());
}

void Renderer::CreateSampler() {
	this->sampler = std::unique_ptr<Sampler>(new Sampler());
	this->sampler->Init(this->device.Get(), D3D11_TEXTURE_ADDRESS_WRAP);

	this->shadowSampler = std::unique_ptr<Sampler>(new Sampler());
	this->shadowSampler->Init(this->device.Get(), D3D11_TEXTURE_ADDRESS_BORDER, D3D11_FILTER_ANISOTROPIC,
							  D3D11_COMPARISON_LESS_EQUAL, {1, 1, 1, 1});
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

void Renderer::CreateRendererConstantBuffers() {
	CameraObject::CameraMatrixContainer camMatrix = {};
	this->cameraBuffer = std::make_unique<ConstantBuffer>();
	this->cameraBuffer->Init(this->device.Get(), sizeof(CameraObject::CameraMatrixContainer), &camMatrix,
							 D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	Renderer::WorldMatrixBufferContainer worldMatrix = {};
	this->worldMatrixBuffer = std::make_unique<ConstantBuffer>();
	this->worldMatrixBuffer->Init(this->device.Get(), sizeof(Renderer::WorldMatrixBufferContainer), &worldMatrix,
								  D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	this->spotlightBuffer = std::make_unique<StructuredBuffer<SpotlightObject::SpotLightContainer>>();
	this->spotlightBuffer->Init(this->device.Get(),this->maximumSpotlights ,{});

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

void Renderer::CreateRenderQueue() {
	// this->meshRenderQueue = std::make_shared<std::vector<std::weak_ptr<MeshObject>>>();
	// this->SpotLightRenderQueue = std::make_shared<std::vector<std::weak_ptr<SpotlightObject>>>();
	// this->renderQueue = std::unique_ptr<RenderQueue>(new RenderQueue(this->meshRenderQueue,
	// this->SpotLightRenderQueue));
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
	BindInputLayout();
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

void Renderer::ChangeSkybox(std::string filepath) 
{
	this->skybox->SwapCubemap(this->device.Get(), this->immediateContext.Get(), filepath);
}

ID3D11Device* Renderer::GetDevice() const
{
	return this->device.Get();
}

ID3D11DeviceContext* Renderer::GetContext() const { return this->immediateContext.Get(); }

IDXGISwapChain* Renderer::GetSwapChain() const { return this->swapChain.Get(); }

void Renderer::RenderPass() {
	const auto start{std::chrono::steady_clock::now()};

	// Bind basic stuff (it probably doesn't need to be set every frame)
	if (!this->hasBoundStatic) {
		BindSampler();
		BindInputLayout();
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
	// The skybox needs to not be wireframe and also correct culling
	BindRasterizerState(this->skyboxRasterizerState.get());
	DrawSkybox();

	// Bind rasterizerState
	if (!this->renderAllWireframe) {
		BindRasterizerState(this->standardRasterizerState.get());
	} else {
		BindMaterial(this->defaultUnlitMat.lock().get());
		BindRasterizerState(this->wireframeRasterizerState.get());
	}

	// World Matrix Buffer only needs to be bound once per frame
	BindWorldMatrix(this->worldMatrixBuffer->GetBuffer());

	const auto afterBinds{std::chrono::steady_clock::now()};
	const std::chrono::duration<double> elapsed_seconds{afterBinds - start};
	ImGui::Text(("Render pass setup: " + std::to_string(elapsed_seconds.count())).c_str());

	// Bind meshes
	for (size_t i = 0; i < this->meshRenderQueue.size(); i++) {
		if (this->meshRenderQueue[i].expired()) {
			// This should get rid of empty objects
			Logger::Log("The renderer deleted a meshObject");
			this->meshRenderQueue.erase(this->meshRenderQueue.begin() + i);
			i--;
			continue;
		}

		std::shared_ptr<MeshObject> meshObject = this->meshRenderQueue[i].lock();

		if (!meshObject->IsActive() || meshObject->IsHidden()) continue;

		RenderMeshObject(meshObject.get());
	}
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
		for (auto& mesh : this->meshRenderQueue) {
			if (mesh.expired()) continue;
			if (!mesh.lock()->IsActive() || mesh.lock()->IsHidden()) continue;
			this->RenderMeshObject(mesh.lock().get(), false);
		}
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
			for (auto& mesh : this->meshRenderQueue) {
				if (mesh.expired()) continue;
				if (!mesh.lock()->IsActive() || mesh.lock()->IsHidden()) continue;
				this->RenderMeshObject(mesh.lock().get(), false);
			}
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

void Renderer::BindInputLayout() {
	// Set up inputs
	this->immediateContext->IASetInputLayout(this->inputLayout->GetInputLayout());
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

	// FIX
	this->immediateContext->PSSetShaderResources(1, renderData.textures.size(), renderData.textures.data());

	// Also bind constant buffers
	for (size_t i = 0; i < renderData.pixelBuffers.size(); i++) {
		ID3D11Buffer* buf = renderData.pixelBuffers[i]->GetBuffer();
		this->immediateContext->PSSetConstantBuffers(i + 1, 1, &buf); // i + 1 because first slot is always occupied
	}

	for (size_t i = 0; i < renderData.pixelBuffers.size(); i++) {
		ID3D11Buffer* buf = renderData.pixelBuffers[i]->GetBuffer();
		this->immediateContext->VSSetConstantBuffers(i + 2, 1,
													 &buf); // i + 2 because the first two slots are always occupied
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

	if (mesh.get() != this->currentMesh) {
		VertexBuffer vBuf = mesh->GetVertexBuffer();

		UINT stride = vBuf.GetVertexSize();
		UINT offset = 0;
		ID3D11Buffer* vBuff = vBuf.GetBuffer();
		this->immediateContext->IASetVertexBuffers(0, 1, &vBuff, &stride, &offset);
		this->immediateContext->IASetIndexBuffer(mesh->GetIndexBuffer().GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
		this->currentMesh = mesh.get();
	}

	// Bind worldmatrix
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, meshObject->GetGlobalWorldMatrix(false));
	DirectX::XMFLOAT4X4 worldMatrixInverseTransposed;
	DirectX::XMStoreFloat4x4(&worldMatrixInverseTransposed, meshObject->GetGlobalWorldMatrix(true));

	Renderer::WorldMatrixBufferContainer worldMatrixBufferContainer = {worldMatrix, worldMatrixInverseTransposed};

	this->worldMatrixBuffer->UpdateBuffer(this->immediateContext.Get(), &worldMatrixBufferContainer);
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
			this->immediateContext->DrawIndexed(subMesh.GetNrOfIndices(), subMesh.GetStartIndex(), 0);
		}

		index++;
	}
}
