#pragma once

#include <d3d11.h>

#include "rendering/textureCube.h"
#include "rendering/vertexBuffer.h"
#include "rendering/indexBuffer.h"
#include "rendering/shader.h"
#include "rendering/vertex.h"
#include "core/assetManager.h"

class Skybox {
public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* context, std::string filepath);

	/// <summary>
	/// Delete the old texturecube and create a new one from a file
	/// </summary>
	/// <param name="device"></param>
	/// <param name="context"></param>
	/// <param name="filepath"></param>
	void SwapCubemap(ID3D11Device* device, ID3D11DeviceContext* context, std::string filepath);

	/// <summary>
	/// Draws the cubemap to screen. Make sure the correct rasterizerstate and depthstencilstate is bound.
	/// </summary>
	/// <param name="context"></param>
	void Draw(ID3D11DeviceContext* context);

private:
	std::unique_ptr<TextureCube> textureCube;

	std::unique_ptr<VertexBuffer> vertexBuffer;
	std::unique_ptr<IndexBuffer> indexBuffer;

	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> pixelShader;
};