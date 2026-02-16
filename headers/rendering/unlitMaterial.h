#pragma once

#include "rendering/baseMaterial.h"

#include "rendering/texture.h"
#include <memory>

#include "rendering/shader.h"

class UnlitMaterial : public BaseMaterial {
public:
	std::weak_ptr<Shader> unlitShader;
	float color[4];
	std::shared_ptr<Texture> diffuseTexture;

	std::unique_ptr<ConstantBuffer> materialInfo;

	UnlitMaterial(ID3D11Device* device) : BaseMaterial(device) {
		for (size_t i = 0; i < 4; i++) {
			this->color[i] = 1.0;
		}

		BaseMaterial::BasicMaterialStruct data = {{0.3, 0.3, 0.3, 0.3}, {1, 1, 1, 1}, {1, 1, 1, 1}, 100, 0, {0, 0}};
		this->materialInfo = std::make_unique<ConstantBuffer>();
		this->materialInfo->Init(device, sizeof(BaseMaterial::BasicMaterialStruct), &data, D3D11_USAGE_DYNAMIC,
								 D3D11_CPU_ACCESS_WRITE);
	}

	RenderData GetRenderData(ID3D11DeviceContext* context) override {
		BaseMaterial::BasicMaterialStruct newData = {{0.3, 0.3, 0.3, 0.3},
													 {color[0], color[1], color[2], color[3]},
													 {1, 1, 1, 1},
													 100,
													 this->diffuseTexture ? TextureSlots::first : TextureSlots::none,
													 {0, 0}};

		this->materialInfo->UpdateBuffer(context, &newData);

		return RenderData{
			.pixelShader{this->unlitShader},
			.vertexShader{nullptr},
			.pixelBuffers{this->materialInfo.get()},
			.vertexBuffers{},
			.textures{
				this->diffuseTexture ? this->diffuseTexture->GetSrv() : nullptr,
			},
		};
	}
};