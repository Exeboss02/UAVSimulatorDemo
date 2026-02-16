#pragma once

#include "rendering/constantBuffer.h"
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "utilities/logger.h"
#include <vector>

class Material {
private:
public:
	struct BasicMaterialStruct {
		float ambient[4];
		float diffuse[4];
		float specular[4];
		float shininess;
		int textureCount;
		float padding[2];
	};

	bool wireframe = false;

	Material() = default;
	~Material() = default;
	Material(Material&&) = default;

	void Init(std::shared_ptr<Shader>& vShader, std::shared_ptr<Shader>& pShader);

	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> pixelShader;

	std::vector<std::unique_ptr<ConstantBuffer>> vertexShaderBuffers;
	std::vector<std::unique_ptr<ConstantBuffer>> pixelShaderBuffers;

	std::vector<std::shared_ptr<Texture>> textures;

	std::string identifier;
};