#pragma once

#include "rendering/baseMaterial.h"
#include "rendering/texture.h"

#include <memory>

class GenericMaterial : public BaseMaterial {
public:
	std::shared_ptr<Texture> diffuseTexture;
	std::shared_ptr<Texture> ambientTexture;
	std::shared_ptr<Texture> specularTexture;
	std::shared_ptr<Texture> normalTexture;

	std::unique_ptr<ConstantBuffer> materialInfo;

	BaseMaterial::BasicMaterialStruct basicData;

	GenericMaterial(ID3D11Device* device) : BaseMaterial(device) {
		BaseMaterial::BasicMaterialStruct data = {{0.1, 0.1, 0.1, 0.1}, {1, 1, 1, 1}, {1, 1, 1, 1}, 100, 0, {0, 0}};
		basicData = data;
		this->materialInfo = std::make_unique<ConstantBuffer>();
		this->materialInfo->Init(device, sizeof(BaseMaterial::BasicMaterialStruct), &data, D3D11_USAGE_DYNAMIC,
								 D3D11_CPU_ACCESS_WRITE);
	}

	RenderData GetRenderData(ID3D11DeviceContext* context) override {
		unsigned int newTextureSlots = 0;
		newTextureSlots += this->diffuseTexture ? TextureSlots::first : TextureSlots::none;
		newTextureSlots += this->ambientTexture ? TextureSlots::second : TextureSlots::none;
		newTextureSlots += this->specularTexture ? TextureSlots::third : TextureSlots::none;
		newTextureSlots += this->normalTexture ? TextureSlots::fourth : TextureSlots::none;

		basicData.textureSlots = newTextureSlots;

		this->materialInfo->UpdateBuffer(context, &basicData);

		return RenderData{
			.pixelShader{nullptr},
			.vertexShader{nullptr},
			.pixelBuffers{this->materialInfo.get()},
			.vertexBuffers{},
			.textures{
				this->diffuseTexture ? this->diffuseTexture->GetSrv() : nullptr,
				this->ambientTexture ? this->ambientTexture->GetSrv() : nullptr,
				this->specularTexture ? this->specularTexture->GetSrv() : nullptr,
				this->normalTexture ? this->normalTexture->GetSrv() : nullptr,
			},
		};
	}
};