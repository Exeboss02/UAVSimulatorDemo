#pragma once

#include "core/assetManager.h"
#include "core/window.h"
#include "d3d11.h"
#include "gameObjects/cameraObject.h"
#include "gameObjects/meshObject.h"
#include "gameObjects/spotlightObject.h"
#include "rendering/constantBuffer.h"
#include "rendering/depthBuffer.h"
#include "rendering/indexBuffer.h"
#include "rendering/inputLayout.h"
#include "rendering/material.h"
#include "rendering/rasterizerState.h"
#include "rendering/renderQueue.h"
#include "rendering/renderTarget.h"
#include "rendering/sampler.h"
#include "rendering/shader.h"
#include "rendering/structuredBuffer.h"
#include "rendering/vertex.h"
#include "rendering/vertexBuffer.h"
#include "wrl/client.h"
#include <algorithm>

class Renderer {
public:
	Renderer();
	~Renderer() = default;

	/// <summary>
	/// Initialize the renderer
	/// </summary>
	/// <param name="window"></param>
	void Init(const Window& window);

	/// <summary>
	/// Set all default material stuff,
	/// and set some stuff that requires defaults
	/// </summary>
	void SetAllDefaults();

	/// <summary>
	/// Render a frame
	/// </summary>
	void Render();

	/// <summary>
	/// Present the swapchain to the window
	/// </summary>
	void Present();

	/// <summary>
	/// Resizes swapchain to fit window
	/// </summary>
	/// <param name="window"></param>
	void Resize(const Window& window);

	/// <summary>
	/// Toggles VSync
	/// </summary>
	/// <param name="enable"></param>
	void ToggleVSync(bool enable);

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetContext() const;
	IDXGISwapChain* GetSwapChain() const;

private:
	const size_t maximumSpotlights;

	struct WorldMatrixBufferContainer {
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT4X4 worldMatrixInversedTransposed;
	};

	struct LightCountBufferContainer {
		uint32_t spotlightCount;
		float padding[3];
	};

	// DirectX11 specific stuff:

	D3D11_VIEWPORT viewport;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

	std::unique_ptr<RenderTarget> renderTarget;
	std::unique_ptr<DepthBuffer> depthBuffer;
	std::unique_ptr<InputLayout> inputLayout;
	std::unique_ptr<Sampler> sampler;
	std::unique_ptr<RasterizerState> standardRasterizerState;
	std::unique_ptr<RasterizerState> wireframeRasterizerState;
	RasterizerState* currentRasterizerState;

	// Default stuff
	// Avoids calling the assetmanager every frame

	std::weak_ptr<BaseMaterial> defaultMat;
	std::weak_ptr<BaseMaterial> defaultUnlitMat;

	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> pixelShaderLit;
	std::shared_ptr<Shader> pixelShaderUnlit;

	Shader* currentPixelShader;
	Shader* currentVertexShader;

	// Render Queue:

	std::unique_ptr<RenderQueue> renderQueue;
	std::shared_ptr<std::vector<std::weak_ptr<MeshObject>>> meshRenderQueue;
	std::shared_ptr<std::vector<std::weak_ptr<SpotlightObject>>> lightRenderQueue;

	// Constant buffers:
	// The renderer keeps these constant buffers since only one is ever required
	// So it just updates them with data for each object every frame

	std::unique_ptr<ConstantBuffer> cameraBuffer;
	std::unique_ptr<ConstantBuffer> worldMatrixBuffer;

	std::unique_ptr<ConstantBuffer> spotlightCountBuffer;
	std::unique_ptr<StructuredBuffer> spotlightBuffer;

	// ImGui variables

	bool isVSyncEnabled = false;
	bool renderAllWireframe = false;
	bool hasBoundStatic = false;

	void SetViewport(const Window& window);
	void CreateDeviceAndSwapChain(const Window& window);
	void CreateRenderTarget();
	void CreateDepthBuffer(const Window& window);
	void CreateInputLayout(const std::string& vShaderByteCode);
	void CreateSampler();
	void CreateRasterizerStates();

	/// <summary>
	/// Creates required constant buffers. The renderer needs a cameraBuffer and worldMatrixBuffer.
	/// </summary>
	void CreateRendererConstantBuffers();

	void CreateRenderQueue();

	void LoadShaders();

	/// <summary>
	/// This is where the actual rendering logic is done
	/// </summary>
	void RenderPass();

	/// <summary>
	/// Clears last frame with a clear color
	/// </summary>
	void ClearRenderTargetViewAndDepthStencilView();

	void ResizeSwapChain(const Window& window);

	void BindSampler();
	void BindInputLayout();
	void BindRenderTarget();
	void BindViewport();
	void BindRasterizerState(RasterizerState* rastState);

	void BindMaterial(BaseMaterial* material);
	void BindLights();

	void BindCameraMatrix();
	void BindWorldMatrix(ID3D11Buffer* buffer);

	/// <summary>
	/// Renders a single MeshObject
	/// </summary>
	/// <param name="meshObject"></param>
	void RenderMeshObject(MeshObject* meshObject);
};