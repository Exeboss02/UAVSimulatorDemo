#include "rendering/skybox.h"

void Skybox::Init(ID3D11Device* device, ID3D11DeviceContext* context, std::string filepath) {
	SwapCubemap(device, context, filepath);

	Vertex vertexData[] = {{-1, -1, -1, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f}, {1, -1, -1, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f},
						   {-1, 1, -1, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},	{1, 1, -1, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f},

						   {-1, -1, 1, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f},	{1, -1, 1, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f},
						   {-1, 1, 1, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},	{1, 1, 1, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f}};

	uint32_t indexData[] = {// Front face (-Z)
							0, 1, 2, 2, 1, 3,

							// Back face (+Z)
							5, 4, 7, 7, 4, 6,

							// Left face (-X)
							4, 0, 6, 6, 0, 2,

							// Right face (+X)
							1, 5, 3, 3, 5, 7,

							// Top face (+Y)
							2, 3, 6, 6, 3, 7,

							// Bottom face (-Y)
							4, 5, 0, 0, 5, 1};

	this->vertexBuffer = std::make_unique<VertexBuffer>();
	this->vertexBuffer->Init(device, sizeof(Vertex), 8, vertexData);

	this->indexBuffer = std::make_unique<IndexBuffer>();
	this->indexBuffer->Init(device, 36, indexData);

	this->vertexShader = AssetManager::GetInstance().GetShaderPtr("VSSkybox");
	this->pixelShader = AssetManager::GetInstance().GetShaderPtr("PSSkybox");
}

void Skybox::SwapCubemap(ID3D11Device* device, ID3D11DeviceContext* context, std::string filepath) 
{
	this->textureCube = std::make_unique<TextureCube>();
	this->textureCube->Init(device, context, filepath);
}

void Skybox::Draw(ID3D11DeviceContext* context) {
	// Bind shaders
	this->vertexShader->BindShader(context);
	this->pixelShader->BindShader(context);

	// Bind vertex- and indexbuffers
	UINT stride = this->vertexBuffer->GetVertexSize();
	UINT offset = 0;
	ID3D11Buffer* vBuff = this->vertexBuffer->GetBuffer();
	context->IASetVertexBuffers(0, 1, &vBuff, &stride, &offset);
	context->IASetIndexBuffer(this->indexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

	// Bind texture
	ID3D11ShaderResourceView* srv = this->textureCube->GetShaderResourceView();
	context->PSSetShaderResources(1, 1, &srv);

	// Draw
	context->DrawIndexed(36, 0, 0);

	// Get rid of the texture cube since it causes errors
	ID3D11ShaderResourceView* nullSrv = nullptr;
	context->PSSetShaderResources(1, 1, &nullSrv);
}
